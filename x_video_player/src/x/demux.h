//
// Created by dingrui on 2/27/26.
//

#pragma once

#include "ffmpeg_raii.h"
#include "pch.h"

extern "C" {
#include <libavcodec/avcodec.h>
}

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
     * 获取音频参数
     */
    AVCodecParametersPtr CopyAPara() {
        return this->copyPara(m_aStream);
    }

    /**
     * 获取视频参数
     */
    AVCodecParametersPtr CopyVPara() {
        return this->copyPara(m_vStream);
    }

    /**
     * @param pos 位置 [0.0, 1.0]
     */
    virtual bool Seek(double pos);

    /**
     * 清空读取缓存
     */
    void Clear();
    void Close();

private:
    AVCodecParametersPtr copyPara(int streamIndex);

protected:
    AVFormatContext* m_ic{nullptr};
    int m_aStream{-1};
    int m_vStream{-1};
    int m_totalMs{0};

    std::mutex m_mutex;
};
