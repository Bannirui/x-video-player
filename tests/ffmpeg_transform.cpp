#include "x.h"

extern "C"
{
#include <libavformat/avformat.h>
#include <libavcodec/avcodec.h>
#include <libswscale/swscale.h>
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

    int videoStream = -1, audioStream = -1;
    // audio/video stream
    audioStream = av_find_best_stream(context, AVMEDIA_TYPE_AUDIO, -1, -1, nullptr, 0);
    videoStream = av_find_best_stream(context, AVMEDIA_TYPE_VIDEO, -1, -1, nullptr, 0);
    XLOG_INFO("audio index={}, video index={}", audioStream, videoStream);

    // video codec
    const AVCodec *vCodec = avcodec_find_decoder(context->streams[videoStream]->codecpar->codec_id);
    if (!vCodec)
    {
        XLOG_ERROR("cannot find video decoder");
        return -1;
    }
    XLOG_INFO("find the video codec context, {}", (int)(context->streams[videoStream]->codecpar->codec_id));
    // create codec context
    AVCodecContext *vCodecCtx = avcodec_alloc_context3(vCodec);
    // copy codec-context parameters
    avcodec_parameters_to_context(vCodecCtx, context->streams[videoStream]->codecpar);
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
    const AVCodec *aCodec = avcodec_find_decoder(context->streams[audioStream]->codecpar->codec_id);
    if (!aCodec)
    {
        XLOG_ERROR("cannot find audio decoder");
        return -1;
    }
    XLOG_INFO("find the audio codec context, {}", (int)(context->streams[audioStream]->codecpar->codec_id));
    // create codec context
    AVCodecContext *aCodecCtx = avcodec_alloc_context3(aCodec);
    // copy codec-context parameters
    avcodec_parameters_to_context(aCodecCtx, context->streams[audioStream]->codecpar);
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
    AVPacket *pkt   = av_packet_alloc();
    AVFrame  *frame = av_frame_alloc();
    // pixel format and size transform
    SwsContext    *transformCtx = nullptr;
    unsigned char *rgb          = nullptr;
    for (;;)
    {
        if (av_read_frame(context, pkt) != 0)
        {
            // replay when end
            XLOG_INFO("the end");
            int       ms  = 3000;
            long long pos = ((double)ms / 1000) * r2d(context->streams[pkt->stream_index]->time_base);
            av_seek_frame(context, videoStream, pos, AVSEEK_FLAG_BACKWARD | AVSEEK_FLAG_FRAME);
            continue;
        }
        XLOG_INFO("read, packet size={}, pts={} dts={}", pkt->size, pkt->pts, pkt->dts);
        XLOG_INFO("pkt ms={}", pkt->pts * (r2d(context->streams[pkt->stream_index]->time_base) * 1000));
        AVCodecContext *avCodecCtx = nullptr;
        if (pkt->stream_index == audioStream)
        {
            XLOG_INFO("this frame is audio");
            avCodecCtx = aCodecCtx;
        }
        else if (pkt->stream_index == videoStream)
        {
            XLOG_INFO("this frame is image");
            avCodecCtx = vCodecCtx;
        }
        // decode, send packet to according thread, time free(no cpu)
        ret = avcodec_send_packet(avCodecCtx, pkt);
        // decrease ref, free when ref equals 0
        av_packet_unref(pkt);
        if (ret != 0)
        {
            char buf[1024] = {0};
            av_strerror(ret, buf, sizeof(buf) - 1);
            XLOG_ERROR("av codec send failed: {}", buf);
            continue;
        }
        // get decode result from thread, time free(no cpu), send to recv maybe 1:n
        for (;;)
        {
            ret = avcodec_receive_frame(avCodecCtx, frame);
            if (ret != 0)
            {
                break;
            }
            XLOG_INFO("recv frame, format {0}, line size {1}", frame->format, frame->linesize[0]);
            if (avCodecCtx == vCodecCtx)
            {
                // video
                transformCtx = sws_getCachedContext(
                    transformCtx,                 // it will create transform context if the parameter is NULL
                    frame->width, frame->height,  // input width and height
                    static_cast<AVPixelFormat>(frame->format), frame->width, frame->height,  // output width and height
                    AV_PIX_FMT_RGBA,                                                         // output format, RGBA
                    SWS_BILINEAR, nullptr, nullptr, nullptr);
                if (transformCtx)
                {
                    if (!rgb)
                    {
                        rgb = new unsigned char[frame->width * frame->height * 4];
                    }
                    uint8_t *data[2] = {0};
                    data[0]          = rgb;
                    int lines[2]     = {0};
                    lines[0]         = frame->width * 4;
                    ret              = sws_scale(transformCtx,
                                                 frame->data,      // input data
                                                 frame->linesize,  // input line size
                                                 0,
                                                 frame->height,  // input height
                                                 data,           // ouput data
                                                 lines);
                    XLOG_INFO("wsw_scale={}", ret);
                }
            }
        }
    }
    av_frame_free(&frame);
    av_packet_free(&pkt);

    if (context)
    {
        // free the context and then assign content nullptr
        avformat_close_input(&context);
    }
    return 0;
}
