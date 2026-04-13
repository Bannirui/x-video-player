//
// Created by dingrui on 2/27/26.
//

#include "x/demux.h"

#include "x/x_log.h"

extern "C" {
#include <libavcodec/avcodec.h>
#include <libavcodec/codec_par.h>
#include <libavformat/avformat.h>
}

// ffmpeg怕精度丢失 就直接给我们分子分母 本质就是一个浮点数的表示形式
static double r2d(AVRational r) {
    return r.den == 0 ? 0 : (double)r.num / (double)r.den;
}

/**
 * 根据错误码拿到失败原因
 * @param errNum ffmpeg的错误码
 */
static void printErrMsg(int errNum) {
    char buf[1024] = {0};
    // 拿到失败原因
    av_strerror(errNum, buf, sizeof(buf) - 1);
    XLOG_INFO("err: {}", buf);
}

Demux::Demux() {
    static bool s_initialized = false;
    static std::mutex s_mutex;
    s_mutex.lock();
    if (!s_initialized) {
        // 初始化网络库
        avformat_network_init();
        s_initialized = true;
    }
    s_mutex.unlock();
}

Demux::~Demux() {}

bool Demux::Open(const std::string& url) {
    AVDictionary* opts = nullptr;
    // 设置rtsp流用tcp协议打开
    av_dict_set(&opts, "rtsp_transport", "tcp", 0);
    // 网络延时时间
    av_dict_set(&opts, "max_delay", "500", 0);

    m_mutex.lock();
    // 解封装
    int ret = avformat_open_input(&m_avContext, url.c_str(),
                                  nullptr,  // 自动选择解封器
                                  &opts     // 参数设置 比如rtsp的延时时间
    );
    if (ret != 0) {  // 失败
        m_mutex.unlock();
        XLOG_INFO("open {} failed", url);
        printErrMsg(ret);
        return false;
    }
    XLOG_INFO("open {} success", url);
    ret = avformat_find_stream_info(m_avContext, nullptr);
    // 总时长 有这么多个duration 1个duration是1/1'000'000秒 也就是微妙 除以AV_TIME_BASE换算成秒
    m_totalMs = m_avContext->duration / AV_TIME_BASE * 1000;
    XLOG_INFO("total {}ms", m_totalMs);
    // 打印视频流详细信息
    av_dump_format(m_avContext, 0, url.c_str(), 0);

    // 获取流下标
    m_vStream = av_find_best_stream(m_avContext, AVMEDIA_TYPE_VIDEO,
                                    -1,  // -1表示自动选择
                                    -1,  // -1表示none
                                    nullptr, 0);
    AVStream* vStream = m_avContext->streams[m_vStream];
    XLOG_INFO("video, stream:{0}, width:{1}, height:{2}, fps:{3}, format:{4}, codec:{5}", vStream->id,
              vStream->codecpar->width, vStream->codecpar->height, r2d(vStream->avg_frame_rate),
              vStream->codecpar->format, static_cast<int>(vStream->codecpar->codec_id));
    XLOG_INFO("audio index={}, video index={}", m_aStream, m_vStream);

    m_aStream = av_find_best_stream(m_avContext, AVMEDIA_TYPE_AUDIO, -1, -1, nullptr, 0);
    AVStream* aStream = m_avContext->streams[m_aStream];
    XLOG_INFO("audio, stream:{0}, sample rate:{1}, channels:{2}, fps:{3}, format:{4}, codec:{5}", aStream->id,
              aStream->codecpar->sample_rate, aStream->codecpar->channels, r2d(aStream->avg_frame_rate),
              aStream->codecpar->format, static_cast<int>(aStream->codecpar->codec_id));

    m_mutex.unlock();
    return true;
}

AVPacket* Demux::Read() {
    m_mutex.lock();
    if (!m_avContext) {
        m_mutex.unlock();
        return nullptr;
    }
    // 内存的申请跟释放成套使用 av_packet_alloc跟av_packet_free
    AVPacket* pkt = av_packet_alloc();
    // pkt是输出参数 不能是nullptr
    int ret = av_read_frame(m_avContext, pkt);
    if (ret < 0) {  // 报错或者读到文件结尾ret都是小于0
        m_mutex.unlock();
        av_packet_free(&pkt);
        return nullptr;
    }
    // 时间格式转换 s->ms 时间单位转换成ms方便同步
    // 显示时间
    pkt->pts = pkt->pts * r2d(m_avContext->streams[pkt->stream_index]->time_base) * 1000;
    // 解码时间
    pkt->dts = pkt->dts * r2d(m_avContext->streams[pkt->stream_index]->time_base) * 1000;
    XLOG_INFO("pts:{}, dts:{}", pkt->pts, pkt->dts);
    m_mutex.unlock();
    return pkt;
}

AVCodecParameters* Demux::CopyAPara() {
    m_mutex.lock();
    if (!m_avContext) {
        m_mutex.unlock();
        return nullptr;
    }
    AVCodecParameters* ret = avcodec_parameters_alloc();
    avcodec_parameters_copy(ret, m_avContext->streams[m_aStream]->codecpar);
    m_mutex.unlock();
    return ret;
}

AVCodecParameters* Demux::CopyVPara() {
    m_mutex.lock();
    if (!m_avContext) {
        m_mutex.unlock();
        return nullptr;
    }
    AVCodecParameters* ret = avcodec_parameters_alloc();
    avcodec_parameters_copy(ret, m_avContext->streams[m_vStream]->codecpar);
    m_mutex.unlock();
    return ret;
}
