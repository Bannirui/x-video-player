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
    // 打开解码器 找到解码器 参数里面的id
    const AVCodec* avCodec = avcodec_find_decoder(para->codec_id);
    if (!avCodec) {
        XLOG_INFO("could not find codec id {}", static_cast<int>(para->codec_id));
        return false;
    }
    XLOG_INFO("find the AVCodec={}", static_cast<int>(para->codec_id));
    AVCodecContext* vc = avcodec_alloc_context3(avCodec);
    // 配置解码器上下文
    avcodec_parameters_to_context(vc, para.get());
    // 几个线程解码
    vc->thread_count = 8;
    // 打开解码器上下文
    int ret = avcodec_open2(vc, nullptr, nullptr);
    if (ret != 0) {
        avcodec_free_context(&vc);
        LogAvErr(ret, "Could not open codec");
        return false;
    }
    XLOG_INFO("avcodec_open2 success");
    return true;
}
