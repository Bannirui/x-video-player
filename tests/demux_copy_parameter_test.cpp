//
// Created by dingrui on 2/27/26.
//

#include "x/x_log.h"
#include "x/demux.h"

extern "C"
{
#include <libavformat/avformat.h>
#include <libavcodec/avcodec.h>
#include <libavcodec/codec_par.h>
}

int main()
{
    XLog::Init();

    Demux       demux;
    std::string path = "asset/Python.mp4";
    bool        ret  = demux.Open(path);
    XLOG_INFO("ret: {0}", ret);

    AVCodecParameters *aParameter = demux.CopyAPara();
    AVCodecParameters *vParameter = demux.CopyVPara();
    XLOG_INFO("audio parameter: {}", static_cast<int>(aParameter->codec_id));
    XLOG_INFO("video parameter: {}", static_cast<int>(vParameter->codec_id));
    avcodec_parameters_free(&aParameter);
    avcodec_parameters_free(&vParameter);

    return 0;
}