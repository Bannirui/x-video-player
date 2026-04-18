//
// Created by dingrui on 4/18/26.
//

#include "decode.h"

#include "ffmpeg_utils.h"
#include "x/x_log.h"

extern "C" {
#include <libavcodec/avcodec.h>
#include <libavcodec/codec_par.h>
#include <libavformat/avformat.h>
}

Decode::Decode() {}

Decode::~Decode() {}

bool Decode::Open(AVCodecParametersPtr para) {
    if (!para) return false;
    this->Close();
    // 打开解码器 找到解码器 参数里面的id
    const AVCodec* avCodec = avcodec_find_decoder(para->codec_id);
    if (!avCodec) {
        XLOG_INFO("could not find codec id {}", static_cast<int>(para->codec_id));
        return false;
    }
    XLOG_INFO("find the AVCodec={}", static_cast<int>(para->codec_id));
    std::lock_guard<std::mutex> lock(m_mutex);
    m_codec = avcodec_alloc_context3(avCodec);
    // 配置解码器上下文
    avcodec_parameters_to_context(m_codec, para.get());
    // 几个线程解码
    m_codec->thread_count = 8;
    // 打开解码器上下文
    int ret = avcodec_open2(m_codec, nullptr, nullptr);
    if (ret != 0) {
        avcodec_free_context(&m_codec);
        LogAvErr(ret, "Could not open codec");
        return false;
    }
    XLOG_INFO("avcodec_open2 success");
    return true;
}

bool Decode::Send(AVPacket* pkt) {
    if (!pkt || pkt->size <= 0 || !pkt->data) return false;
    std::lock_guard<std::mutex> lock(m_mutex);
    if (!m_codec) return false;
    int ret = avcodec_send_packet(m_codec, pkt);
    av_packet_free(&pkt);
    return ret == 0;
}

AVFrame* Decode::Recv() {
    if (!m_codec) return nullptr;
    std::lock_guard<std::mutex> lock(m_mutex);
    AVFrame* frame = av_frame_alloc();
    if (avcodec_receive_frame(m_codec, frame) != 0) {
        av_frame_free(&frame);
        return nullptr;
    }
    return frame;
}

void Decode::Clear() {
    std::lock_guard<std::mutex> lock(m_mutex);
    if (m_codec) {
        // 清理解码缓冲
        avcodec_flush_buffers(m_codec);
    }
}

void Decode::Close() {
    std::lock_guard<std::mutex> lock(m_mutex);
    if (m_codec) {
        avcodec_close(m_codec);
        avcodec_free_context(&m_codec);
    }
}
