//
// Created by dingrui on 2/27/26.
//

#include "x/demux.h"

#include "x/x_log.h"

extern "C"
{
#include <libavformat/avformat.h>
}

static double r2d(AVRational r)
{
    return r.den == 0 ? 0 : (double)r.num / (double)r.den;
}

Demux::Demux()
{
    static bool       s_initialized = false;
    static std::mutex s_mutex;
    s_mutex.lock();
    if (!s_initialized)
    {
        avformat_network_init();
        s_initialized = true;
    }
    s_mutex.unlock();
}

Demux::~Demux() {}

bool Demux::Open(const std::string &url)
{
    AVDictionary *opts = nullptr;
    av_dict_set(&opts, "rtsp_transport", "tcp", 0);
    av_dict_set(&opts, "max_delay", "500", 0);

    m_mutex.lock();
    int ret = avformat_open_input(&m_avContext, url.c_str(),
                                  0,     // select codec automatically
                                  &opts  // options, like rtsp delay duration
    );
    if (ret != 0)
    {
        m_mutex.unlock();
        char buf[1024] = {0};
        av_strerror(ret, buf, sizeof(buf) - 1);
        XLOG_INFO("open {} failed, err: {}", url, buf);
        return false;
    }
    XLOG_INFO("open {} success", url);
    ret = avformat_find_stream_info(m_avContext, nullptr);
    // ms
    m_totalMs = m_avContext->duration / (AV_TIME_BASE / 1000);
    XLOG_INFO("total {}ms", m_totalMs);
    // mp4 info
    av_dump_format(m_avContext, 0, url.c_str(), 0);

    // audio/video stream
    m_aStream = av_find_best_stream(m_avContext, AVMEDIA_TYPE_AUDIO, -1, -1, nullptr, 0);
    m_vStream = av_find_best_stream(m_avContext, AVMEDIA_TYPE_VIDEO, -1, -1, nullptr, 0);
    XLOG_INFO("audio index={}, video index={}", m_aStream, m_vStream);

    AVStream *aStream = m_avContext->streams[m_aStream];
    XLOG_INFO("audio, stream:{0}, sample rate:{1}, channels:{2}, fps:{3}, format:{4}, codec:{5}", aStream->id,
              aStream->codecpar->sample_rate, aStream->codecpar->channels, r2d(aStream->avg_frame_rate),
              aStream->codecpar->format, static_cast<int>(aStream->codecpar->codec_id));

    AVStream *vStream = m_avContext->streams[m_vStream];
    XLOG_INFO("video, stream:{0}, width:{1}, height:{2}, fps:{3}, format:{4}, codec:{5}", vStream->id,
              vStream->codecpar->width, vStream->codecpar->height, r2d(vStream->avg_frame_rate),
              vStream->codecpar->format, static_cast<int>(vStream->codecpar->codec_id));

    m_mutex.unlock();
    return true;
}
