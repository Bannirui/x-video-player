//
// Created by dingrui on 2/27/26.
//

#include "x/demux.h"

#include "ffmpeg_utils.h"
#include "x/x_log.h"

extern "C" {
#include <libavcodec/avcodec.h>
#include <libavcodec/codec_par.h>
#include <libavformat/avformat.h>
}

/**
 * @param r ffmpeg怕精度丢失 就直接给我们分子分母 本质就是一个浮点数的表示形式
 * @return 单位s
 */
static double r2d(AVRational r) {
    return r.den == 0 ? 0 : (double)r.num / (double)r.den;
}

Demux::Demux() {
    // 确保只初始化一次
    static std::once_flag s_flag;
    std::call_once(s_flag, []() -> void {
        // 初始化网络库
        avformat_network_init();
    });
}

Demux::~Demux() {}

bool Demux::Open(const std::string& url) {
    this->Close();
    // 保证线程安全
    std::lock_guard<std::mutex> lock(m_mutex);
    AVDictionary* opts = nullptr;
    // 设置rtsp流用tcp协议打开
    av_dict_set(&opts, "rtsp_transport", "tcp", 0);
    // 网络延时时间
    av_dict_set(&opts, "max_delay", "500", 0);
    // 打开输入流 解封装
    int ret = avformat_open_input(&m_ic, url.c_str(),
                                  nullptr,  // 自动选择解封器
                                  &opts     // 参数设置 比如rtsp的延时时间
    );
    if (ret != 0) {  // 失败
        XLOG_INFO("open {} failed", url);
        LogAvErr(ret, "open failed");
        return false;
    }
    XLOG_INFO("open {} success", url);
    ret = avformat_find_stream_info(m_ic, nullptr);
    // 总时长 有这么多个duration 1个duration是1/1'000'000秒 也就是微妙 除以AV_TIME_BASE换算成秒
    m_totalMs = m_ic->duration / AV_TIME_BASE * 1000;
    XLOG_INFO("total {}ms", m_totalMs);
    // 打印视频流详细信息
    av_dump_format(m_ic, 0, url.c_str(), 0);

    // 获取流下标
    m_vStream = av_find_best_stream(m_ic, AVMEDIA_TYPE_VIDEO,
                                    -1,  // -1表示自动选择
                                    -1,  // -1表示none
                                    nullptr, 0);
    if (m_vStream < 0) {  // 如果未找到视频流
        XLOG_INFO("Failed to find video stream in {}", url);
        return false;
    }
    AVStream* vStream = m_ic->streams[m_vStream];
    XLOG_INFO("video, stream:{0}, width:{1}, height:{2}, fps:{3}, format:{4}, codec:{5}", vStream->id,
              vStream->codecpar->width, vStream->codecpar->height, r2d(vStream->avg_frame_rate),
              vStream->codecpar->format, static_cast<int>(vStream->codecpar->codec_id));

    m_aStream = av_find_best_stream(m_ic, AVMEDIA_TYPE_AUDIO, -1, -1, nullptr, 0);
    if (m_aStream < 0) {  // 如果未找到音频流
        XLOG_INFO("Failed to find audio stream in {}", url);
        return false;
    }
    AVStream* aStream = m_ic->streams[m_aStream];
    XLOG_INFO("audio, stream:{0}, sample rate:{1}, channels:{2}, fps:{3}, format:{4}, codec:{5}", aStream->id,
              aStream->codecpar->sample_rate, aStream->codecpar->channels, r2d(aStream->avg_frame_rate),
              aStream->codecpar->format, static_cast<int>(aStream->codecpar->codec_id));

    XLOG_INFO("audio index={}, video index={}", m_aStream, m_vStream);
    return true;
}

AVPacket* Demux::Read() {
    // 保证线程安全
    std::lock_guard<std::mutex> lock(m_mutex);
    if (!m_ic) {
        return nullptr;
    }
    // 内存的申请跟释放成套使用 av_packet_alloc跟av_packet_free
    AVPacket* pkt = av_packet_alloc();
    // pkt是输出参数 不能是nullptr 读取一帧并分配空间
    int ret = av_read_frame(m_ic, pkt);
    if (ret < 0) {  // 报错或者读到文件结尾ret都是小于0
        // 防止内存泄露
        av_packet_free(&pkt);
        return nullptr;
    }
    // 时间格式转换 s->ms 时间单位转换成ms方便同步
    // 显示时间
    pkt->pts = pkt->pts * r2d(m_ic->streams[pkt->stream_index]->time_base) * 1000;
    // 解码时间
    pkt->dts = pkt->dts * r2d(m_ic->streams[pkt->stream_index]->time_base) * 1000;
    XLOG_INFO("pts:{}, dts:{}", pkt->pts, pkt->dts);
    return pkt;
}

bool Demux::Seek(double pos) {
    std::lock_guard<std::mutex> lock(m_mutex);
    if (!m_ic) return false;
    // 清理读取缓冲 seek到新的位置 防止之前有传冲
    avformat_flush(m_ic);
    long long seekPos = 0;
    seekPos = m_ic->streams[m_vStream]->duration * pos;
    int ret = av_seek_frame(m_ic, m_vStream, seekPos, AVSEEK_FLAG_BACKWARD | AVSEEK_FLAG_FRAME);
    return ret >= 0;
}

void Demux::Clear() {
    std::lock_guard<std::mutex> lock(m_mutex);
    if (!m_ic) return;
    avformat_flush(m_ic);
}

void Demux::Close() {
    std::lock_guard<std::mutex> lock(m_mutex);
    if (!m_ic) return;
    avformat_close_input(&m_ic);
    // 清空成员
    m_totalMs = 0;
}

AV_TYPE Demux::getAVType(AVPacket* pkt) const {
    if (!pkt) return AV_TYPE::kUNKNOWN;
    return pkt->stream_index == m_vStream ? AV_TYPE::kVIDEO : AV_TYPE::kAUDIO;
}

AVCodecParametersPtr Demux::copyPara(int streamIndex) {
    // 自动加锁并解锁
    std::lock_guard<std::mutex> lock(m_mutex);
    if (!m_ic || streamIndex < 0 || streamIndex >= static_cast<int>(m_ic->nb_streams)) {
        // 返回空指针并指定自定义删除器
        return {nullptr, avcodec_parameters_deleter};
    }
    // 分配内存
    auto ptr = AVCodecParametersPtr(avcodec_parameters_alloc(), avcodec_parameters_deleter);
    if (!ptr) {
        // 分配失败 返回空指针
        return ptr;
    }
    // 拷贝流的参数
    if (avcodec_parameters_copy(ptr.get(), m_ic->streams[streamIndex]->codecpar) < 0) {
        return {nullptr, avcodec_parameters_deleter};
    }
    // 返回一个智能指针 自动管理内存
    return ptr;
}