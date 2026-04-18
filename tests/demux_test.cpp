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
    XLOG_INFO("open ret: {0}", demux.Open(path));

    Demux::AVCodecParametersPtr aParameter = demux.CopyAPara();
    if (aParameter) {
        XLOG_INFO("audio parameter: {}", static_cast<int>(aParameter->codec_id));
    }

    Demux::AVCodecParametersPtr vParameter = demux.CopyVPara();
    if (vParameter) {
        XLOG_INFO("video parameter: {}", static_cast<int>(vParameter->codec_id));
    }

    XLOG_INFO("seek={}", demux.Seek(0.5));

    for (;;) {
        AVPacket* pkt = demux.Read();
        if (!pkt) break;
    }
    demux.Clear();
    demux.Close();

    return 0;
}