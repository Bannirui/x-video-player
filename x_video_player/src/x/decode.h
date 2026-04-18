//
// Created by dingrui on 4/18/26.
//
#pragma once

#include "ffmpeg_raii.h"
#include "pch.h"

struct AVCodecParameters;
struct AVCodecContext;

class Decode {
public:
    Decode();
    ~Decode();

    /**
     * 打开解码器
     * @param para demux负责创建 我负责接管
     */
    bool Open(AVCodecParametersPtr para);

    void Clear();
    void Close();

private:
    enum class TYPE { AUDIO, VIDEO };
    TYPE m_type{TYPE::AUDIO};

    AVCodecContext* m_codec{nullptr};
    std::mutex m_mutex;
};
