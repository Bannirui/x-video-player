//
// Created by dingrui on 2/27/26.
//

#pragma once

#include "pch.h"

struct AVFormatContext;
struct AVPacket;

class Demux
{
public:
    Demux();
    virtual ~Demux();

    /**
     * @param url media file path
     */
    virtual bool Open(const std::string &url);
    // read frame, caller free AVPacket object, av_paket_free
    virtual AVPacket *Read();

protected:
    AVFormatContext *m_avContext{nullptr};
    int              m_aStream{0};
    int              m_vStream{0};
    int              m_totalMs{0};

    std::mutex m_mutex;
};
