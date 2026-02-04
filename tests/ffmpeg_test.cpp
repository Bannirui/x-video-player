//
// Created by dingrui on 2/1/26.
//

#include "x_log.h"

extern "C" {
#include <libavformat/avformat.h>
}

static double r2d(AVRational r) {
    return r.den == 0 ? 0 : (double) r.num / (double) r.den;
}

int main(int argc, char **argv) {
    XLog::Init();

    avformat_network_init();
    AVFormatContext *context = nullptr;
    const char *path = "asset/Python.mp4";
    AVDictionary *opts = nullptr;
    av_dict_set(&opts, "rtsp_transport", "tcp", 0);
    av_dict_set(&opts, "max_delay", "500", 0);
    int ret = avformat_open_input(
        &context,
        path,
        0, // select codec automatically
        &opts // options, like rtsp delay duration
    );
    if (ret != 0) {
        char buf[1024] = {0};
        av_strerror(ret, buf, sizeof(buf) - 1);
        XLOG_INFO("open {} failed, err: {}", path, buf);
        return -1;
    }
    XLOG_INFO("open {} success", path);
    ret = avformat_find_stream_info(context, 0);
    // ms
    int total = context->duration / (AV_TIME_BASE / 1000);
    XLOG_INFO("total {}ms", total);
    // mp4 info
    av_dump_format(context, 0, path, 0);

    for (uint32_t i = 0; i < context->nb_streams; ++i) {
        AVStream *stream = context->streams[i];
        if (stream->codecpar->codec_type == AVMEDIA_TYPE_AUDIO) {
            XLOG_INFO("audio, i={}", i);
            XLOG_INFO("sample rate={}", stream->codecpar->sample_rate);
            XLOG_INFO("channels={}", stream->codecpar->channels);
            XLOG_INFO("audio fps={}", r2d(stream->avg_frame_rate));
        } else if (stream->codecpar->codec_type == AVMEDIA_TYPE_VIDEO) {
            XLOG_INFO("video, i={}", i);
            XLOG_INFO("width={}", stream->codecpar->width);
            XLOG_INFO("height={}", stream->codecpar->height);
            XLOG_INFO("video fps={}", r2d(stream->avg_frame_rate));
        }
        XLOG_INFO("format={}", stream->codecpar->format);
        XLOG_INFO("codec id={}", static_cast<int>(stream->codecpar->codec_id));
    }

    if (context) {
        // free the context and then assign content nullptr
        avformat_close_input(&context);
    }
    return 0;
}
