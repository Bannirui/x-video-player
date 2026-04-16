//
// Created by dingrui on 2/27/26.
//

#pragma once

#include "pch.h"

struct AVFormatContext;
struct AVPacket;
struct AVCodecParameters;

// 负责音视频解封装 打开/读
class Demux {
public:
    Demux();
    virtual ~Demux();

    /**
     * @param url media file path
     */
    virtual bool Open(const std::string& url);

    /**
     * 读一帧
     * @return 调用方负责对内存的释放 {@see av_packet_free}
     */
    virtual AVPacket* Read();

    /**
     * audio/video parameter
     * @return memory need be freed, call {@see avcodec_parameters_free}
     */
    AVCodecParameters* CopyAPara();
    AVCodecParameters* CopyVPara();

protected:
    AVFormatContext* m_ic{nullptr};
    int m_aStream{0};
    int m_vStream{0};
    int m_totalMs{0};

    std::mutex m_mutex;
};
