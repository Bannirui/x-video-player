//
// Created by dingrui on 2/27/26.
//

#include "x/decode.h"

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

    auto aParameter = demux.CopyAPara();
    if (aParameter) {
        XLOG_INFO("audio parameter: {}", static_cast<int>(aParameter->codec_id));
    }

    auto vParameter = demux.CopyVPara();
    if (vParameter) {
        XLOG_INFO("video parameter: {}", static_cast<int>(vParameter->codec_id));
    }

    XLOG_INFO("seek={}", demux.Seek(0.5));

    Decode aDecode;
    XLOG_INFO("a decode open={}", aDecode.Open(std::move(aParameter)));
    aDecode.Clear();
    aDecode.Close();

    Decode vDecode;
    XLOG_INFO("v decode open={}", vDecode.Open(std::move(vParameter)));
    vDecode.Clear();
    vDecode.Close();

    // for (;;) {
    //     AVPacket* pkt = demux.Read();
    //     if (!pkt) break;
    // }
    demux.Clear();
    demux.Close();

    return 0;
}