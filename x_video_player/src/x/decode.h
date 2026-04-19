//
// Created by dingrui on 4/18/26.
//
#pragma once

#include "ffmpeg_raii.h"
#include "pch.h"

struct AVCodecParameters;
struct AVCodecContext;
struct AVPacket;
struct AVFrame;

class Decode {
public:
    Decode();
    ~Decode();

    /**
     * 打开解码器
     * @param para demux负责创建 我负责接管
     */
    bool Open(AVCodecParametersPtr para);
    /**
     * 发送到解码线程
     * @param pkt 不管成功与否都要释放pkt的空间
     */
    bool Send(AVPacket* pkt);
    /**
     * 获取解码数据 一次{@see Send}可能需要多次的Recv
     * @return 每次都复制一份空间 内存由调用者负责释放{@see av_frame_free}
     */
    AVFrame* Recv();

    void Clear();
    void Close();

private:
    AVCodecContext* m_codec{nullptr};
    std::mutex m_mutex;
};
