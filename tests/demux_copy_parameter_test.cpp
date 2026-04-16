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

int main() {
    XLog::Init();

    Demux demux;
    std::string path = "asset/Python.mp4";
    bool ret = demux.Open(path);
    XLOG_INFO("ret: {0}", ret);

    Demux::AVCodecParametersPtr aParameter = demux.CopyAPara();
    if (aParameter) {
        XLOG_INFO("audio parameter: {}", static_cast<int>(aParameter->codec_id));
    }

    Demux::AVCodecParametersPtr vParameter = demux.CopyVPara();
    if (vParameter) {
        XLOG_INFO("video parameter: {}", static_cast<int>(vParameter->codec_id));
    }

    return 0;
}