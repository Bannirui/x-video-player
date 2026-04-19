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
    const std::string path = "asset/Python.mp4";
    if (!demux.Open(path)) {
        XLOG_ERROR("failed to open {}", path);
        return -1;
    }

    auto aParameter = demux.CopyAPara();
    auto vParameter = demux.CopyVPara();
    if (aParameter) {
        XLOG_INFO("audio parameter: {}", static_cast<int>(aParameter->codec_id));
    }
    if (vParameter) {
        XLOG_INFO("video parameter: {}", static_cast<int>(vParameter->codec_id));
    }

    XLOG_INFO("seek={}", demux.Seek(0.5));

    Decode aDecode;
    Decode vDecode;
    if (aParameter && aDecode.Open(std::move(aParameter))) {
        XLOG_INFO("a decode open={}", aDecode.Open(std::move(aParameter)));
    }
    if (vParameter && vDecode.Open(std::move(vParameter))) {
        XLOG_INFO("v decode open={}", vDecode.Open(std::move(vParameter)));
    }

    for (;;) {
        AVPacket* pkt = demux.Read();
        if (!pkt) break;
        switch (demux.getAVType(pkt)) {
            case AV_TYPE::kAUDIO: {
                if (aDecode.Send(pkt)) {
                    while (true) {
                        AVFrame* frame = aDecode.Recv();
                        if (!frame) break;
                        XLOG_INFO("audio channels: {}", frame->channels);
                        av_frame_free(&frame);
                    }
                }
                break;
            }
            case AV_TYPE::kVIDEO: {
                if (vDecode.Send(pkt)) {
                    while (true) {
                        AVFrame* frame = aDecode.Recv();
                        if (!frame) break;
                        XLOG_INFO("video channels: {}", frame->channels);
                        av_frame_free(&frame);
                    }
                }
                break;
            }
            default:
                XLOG_INFO("unknown type");
                break;
        }
    }
    demux.Clear();
    demux.Close();

    return 0;
}