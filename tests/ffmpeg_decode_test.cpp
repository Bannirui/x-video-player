#include "x.h"

extern "C"
{
#include <libavformat/avformat.h>
#include <libavcodec/avcodec.h>
}

static double r2d(AVRational r)
{
    return r.den == 0 ? 0 : (double)r.num / (double)r.den;
}

int main()
{
    XLog::Init();

    // init network lib, modern ffmpeg no need registering manually
    avformat_network_init();

    AVFormatContext *context = nullptr;
    std::string      path("asset/Python.mp4");
    AVDictionary    *opts = nullptr;
    av_dict_set(&opts, "rtsp_transport", "tcp", 0);
    av_dict_set(&opts, "max_delay", "500", 0);
    int ret = avformat_open_input(&context, path.c_str(),
                                  0,     // select codec automatically
                                  &opts  // options, like rtsp delay duration
    );
    if (ret != 0)
    {
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
    av_dump_format(context, 0, path.c_str(), 0);

    int videoStreamIndex = 0;
    int audioStreamIndex = 1;
    for (uint32_t i = 0; i < context->nb_streams; ++i)
    {
        AVStream *stream = context->streams[i];
        if (stream->codecpar->codec_type == AVMEDIA_TYPE_AUDIO)
        {
            audioStreamIndex = i;
            XLOG_INFO("audio, i={}", i);
            XLOG_INFO("sample rate={}", stream->codecpar->sample_rate);
            XLOG_INFO("channels={}", stream->codecpar->channels);
            XLOG_INFO("audio fps={}", r2d(stream->avg_frame_rate));
        }
        else if (stream->codecpar->codec_type == AVMEDIA_TYPE_VIDEO)
        {
            videoStreamIndex = i;
            XLOG_INFO("video, i={}", i);
            XLOG_INFO("width={}", stream->codecpar->width);
            XLOG_INFO("height={}", stream->codecpar->height);
            XLOG_INFO("video fps={}", r2d(stream->avg_frame_rate));
        }
        XLOG_INFO("format={}", stream->codecpar->format);
        XLOG_INFO("codec id={}", static_cast<int>(stream->codecpar->codec_id));
    }
    XLOG_INFO("audio index={}, video index={}", audioStreamIndex, videoStreamIndex);

    // video stream
    audioStreamIndex = av_find_best_stream(context, AVMEDIA_TYPE_AUDIO, -1, -1, nullptr, 0);
    videoStreamIndex = av_find_best_stream(context, AVMEDIA_TYPE_VIDEO, -1, -1, nullptr, 0);
    XLOG_INFO("audio index={}, video index={}", audioStreamIndex, videoStreamIndex);

    // video codec
    const AVCodec *vCodec = avcodec_find_decoder(context->streams[videoStreamIndex]->codecpar->codec_id);
    if (!vCodec)
    {
        XLOG_ERROR("cannot find video decoder");
        return -1;
    }
    XLOG_INFO("find the video codec context, {}", (int)(context->streams[videoStreamIndex]->codecpar->codec_id));
    // create codec context
    AVCodecContext *vCodecCtx = avcodec_alloc_context3(vCodec);
    // copy codec-context parameters
    avcodec_parameters_to_context(vCodecCtx, context->streams[videoStreamIndex]->codecpar);
    // threads
    vCodecCtx->thread_count = 8;
    // open codec-context
    ret = avcodec_open2(vCodecCtx, 0, 0);
    if (ret != 0)
    {
        char buf[1024] = {0};
        av_strerror(ret, buf, sizeof(buf) - 1);
        XLOG_ERROR("avcodec_open2 failed: {}", buf);
        return -1;
    }
    XLOG_INFO("video codec open succ");

    // audio codec
    const AVCodec *aCodec = avcodec_find_decoder(context->streams[audioStreamIndex]->codecpar->codec_id);
    if (!aCodec)
    {
        XLOG_ERROR("cannot find audio decoder");
        return -1;
    }
    XLOG_INFO("find the audio codec context, {}", (int)(context->streams[audioStreamIndex]->codecpar->codec_id));
    // create codec context
    AVCodecContext *aCodecCtx = avcodec_alloc_context3(aCodec);
    // copy codec-context parameters
    avcodec_parameters_to_context(aCodecCtx, context->streams[audioStreamIndex]->codecpar);
    // threads
    aCodecCtx->thread_count = 8;
    // open codec-context
    ret = avcodec_open2(aCodecCtx, 0, 0);
    if (ret != 0)
    {
        char buf[1024] = {0};
        av_strerror(ret, buf, sizeof(buf) - 1);
        XLOG_ERROR("avcodec_open2 failed: {}", buf);
        return -1;
    }
    XLOG_INFO("audio codec open succ");

    // read
    AVPacket *pkt = av_packet_alloc();
    for (;;)
    {
        if (av_read_frame(context, pkt) != 0)
        {
            // replay when end
            XLOG_INFO("the end");
            int       ms  = 3000;
            long long pos = ((double)ms / 1000) * r2d(context->streams[pkt->stream_index]->time_base);
            av_seek_frame(context, videoStreamIndex, pos, AVSEEK_FLAG_BACKWARD | AVSEEK_FLAG_FRAME);
            continue;
        }
        XLOG_INFO("read, packet size={}, pts={} dts={}", pkt->size, pkt->pts, pkt->dts);
        XLOG_INFO("pkt ms={}", pkt->pts * (r2d(context->streams[pkt->stream_index]->time_base) * 1000));
        if (pkt->stream_index == audioStreamIndex)
        {
            XLOG_INFO("this frame is audio");
        }
        else if (pkt->stream_index == videoStreamIndex)
        {
            XLOG_INFO("this frame is image");
        }
        // decrease ref, free when ref equals 0
        av_packet_unref(pkt);
    }
    av_packet_free(&pkt);

    if (context)
    {
        // free the context and then assign content nullptr
        avformat_close_input(&context);
    }
    return 0;
}
