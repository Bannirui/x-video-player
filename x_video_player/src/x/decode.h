//
// Created by dingrui on 4/18/26.
//
#pragma once

#include "ffmpeg_raii.h"

struct AVCodecParameters;

class Decode {
public:
    Decode();
    ~Decode();

    /**
     * 打开解码器
     * @param para demux负责创建 我负责接管
     */
    bool Open(AVCodecParametersPtr para);

private:
    enum class TYPE { AUDIO, VIDEO };
    TYPE m_type{TYPE::AUDIO};
};
