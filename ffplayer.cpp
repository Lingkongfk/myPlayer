#include "ffplayer.h"
#include "ffmsg.h"
#include <string.h>
#include <iostream>

void print_error(const char* filename, int err){
    char errbuf[128];
    const char *errbuf_ptr = errbuf;

    if (av_strerror(err, errbuf, sizeof(errbuf)) < 0)
        errbuf_ptr = strerror(AVUNERROR(err));
    av_log(NULL, AV_LOG_ERROR, "%s: %s\n", filename, errbuf_ptr);
}

FFPlayer::FFPlayer() {}

int FFPlayer::create()
{
    std::cout << "ffp_create\n";
    msg_queue_init(&msg_queue_);
    return 0;
}

void FFPlayer::destory()
{
    stream_close();
    msg_queue_destroy(&msg_queue_);
}


int FFPlayer::prepare_async_l(const char *data_source)
{
    input_filename_ = strdup(data_source);
    int ret = stream_open(data_source);
    return ret;
}

int FFPlayer::stream_open(const char *file_name)
{
    //初始化帧队列，包队列，时钟，创建read_thread
    if(SDL_Init(SDL_INIT_VIDEO | SDL_INIT_AUDIO | SDL_INIT_TIMER)){
        av_log(NULL, AV_LOG_FATAL, "Could not initialize SDL - %s\n", SDL_GetError());
        av_log(NULL, AV_LOG_FATAL, "(Did you set the DISPLAY variable?)\n");
        return -1;
    }

    if(frame_queue_init(&pictq, &videoq, VIDEO_PICTURE_QUEUE_SIZE_DEFAULT) < 0){
        goto fail;
    }
    if (frame_queue_init(&sampq, &audioq, SAMPLE_QUEUE_SIZE) < 0){
        goto fail;
    }

    // 初始化Packet包队列
    if (packet_queue_init(&videoq) < 0 ||
        packet_queue_init(&audioq) < 0 )
        goto fail;


    read_thread_ = new std::thread(&FFPlayer::read_thread, this);
    return 0;
fail:
    stream_close();
        return -1;
}

void FFPlayer::stream_close()
{
    abort_request = 1; // 请求退出
    if(read_thread_ && read_thread_->joinable()) {
        read_thread_->join();       // 等待线程退出
    }

    // 关闭解复用器 avformat_close_input(&is->ic);
    // 释放packet队列
    packet_queue_destroy(&videoq);
    packet_queue_destroy(&audioq);
    // 释放frame队列
    frame_queue_destory(&pictq);
    frame_queue_destory(&sampq);

    if(input_filename_) {
        free(input_filename_);
        input_filename_ = NULL;
    }
}

//根据流进行解码线程创建和解码
int FFPlayer::stream_component_open(int stream_index)
{
    AVCodecContext* avctx;
    const AVCodec* codec;
    int sample_rate;
    int nb_channels;
    AVChannelLayout channel_layout;
    int ret = 0;

    //判断stream是否合法
    if(stream_index < 0 || stream_index >= ic->nb_streams){
        return -1;
    }
    avctx = avcodec_alloc_context3(NULL);
    if(!avctx){
        return AVERROR(ENOMEM);
    }

    ret = avcodec_parameters_to_context(avctx, ic->streams[stream_index]->codecpar);
    if(ret < 0){
        goto fail;
    }

    codec = avcodec_find_decoder(avctx->codec_id);
    if(!codec){
        av_log(NULL, AV_LOG_WARNING,
               "No decoder could be found for codec %s\n", avcodec_get_name(avctx->codec_id));
        ret = AVERROR(EINVAL);
        goto fail;
    }

    if((ret = avcodec_open2(avctx, codec, NULL)) < 0){
        goto fail;
    }

    switch(avctx->codec_type){
    case AVMEDIA_TYPE_AUDIO:
        sample_rate = avctx->sample_rate;
        nb_channels = avctx->ch_layout.nb_channels;
        channel_layout = avctx->ch_layout;

        audio_stream = stream_index;
        audio_st = ic->streams[audio_stream];

        //启动解码器线程
        //运行音频输出

        break;
    case AVMEDIA_TYPE_VIDEO:
        video_stream = stream_index;
        video_st = ic->streams[video_stream];

        //初始化ffplay的解码器
        //启动解码线程

        break;
    default:
        break;
    }
fail:
    avcodec_free_context(&avctx);

out:
    return ret;
}

void FFPlayer::stream_component_close(int stream_index)
{
    AVCodecParameters *codecpar;

    if (stream_index < 0 || stream_index >= ic->nb_streams)
        return;
    codecpar = ic->streams[stream_index]->codecpar;

    switch (codecpar->codec_type) {
    case AVMEDIA_TYPE_AUDIO:
        std::cout << __FUNCTION__ << "  AVMEDIA_TYPE_AUDIO\n";
        // 请求终止解码器线程
        // 关闭音频设备
        // 销毁解码器

        // 释放重采样器
        // 释放audio buf
        //        decoder_abort(&is->auddec, &is->sampq); // 解码器线程请求abort的时候有调用 packet_queue_abort
        //        SDL_CloseAudioDevice(audio_dev);
        //        decoder_destroy(&is->auddec);
        //        swr_free(&is->swr_ctx);
        //        av_freep(&is->audio_buf1);
        //        is->audio_buf1_size = 0;
        //        is->audio_buf = NULL;
        break;
    case AVMEDIA_TYPE_VIDEO:
        std::cout << __FUNCTION__ << "  AVMEDIA_TYPE_VIDEO\n";
        // 请求终止解码器线程
        // 关闭音频设备
        // 销毁解码器
        //        decoder_abort(&is->viddec, &is->pictq);
        //        decoder_destroy(&is->viddec);
        break;

    default:
        break;
    }

    //    ic->streams[stream_index]->discard = AVDISCARD_ALL;  // 这个又有什么用?
    switch (codecpar->codec_type) {
    case AVMEDIA_TYPE_AUDIO:
        audio_st = NULL;
        audio_stream = -1;
        break;
    case AVMEDIA_TYPE_VIDEO:
        video_st = NULL;
        video_stream = -1;
        break;
    default:
        break;
    }
}

//这里模拟一下运行
int FFPlayer::read_thread()
{
    int err, i, ret;
    int st_index[AVMEDIA_TYPE_NB];
    AVPacket pkt1;
    AVPacket* pkt = &pkt1;

    memset(st_index, -1, sizeof(st_index));
    video_stream = -1;
    audio_stream = -1;
    eof = 0;

    ic = avformat_alloc_context();
    if(!ic){
        av_log(NULL, AV_LOG_FATAL, "Could not allocate context.\n");
        ret = AVERROR(ENOMEM);
        goto fail;
    }

    err = avformat_open_input(&ic, input_filename_, NULL, NULL);
    if (err < 0) {
        print_error(input_filename_, err);
        ret = -1;
        goto fail;
    }
    ffp_notify_msg1(this, FFP_MSG_OPEN_INPUT);
    std::cout << "read_thread FFP_MSG_OPEN_INPUT " << this << std::endl;

    err = avformat_find_stream_info(ic, NULL);
    if (err < 0) {
        av_log(NULL, AV_LOG_WARNING,
               "%s: could not find codec parameters\n", input_filename_);
        ret = -1;
        goto fail;
    }
    ffp_notify_msg1(this, FFP_MSG_FIND_STREAM_INFO);
    std::cout << "read_thread FFP_MSG_FIND_STREAM_INFO " << this << std::endl;


    st_index[AVMEDIA_TYPE_VIDEO] = av_find_best_stream(ic, AVMEDIA_TYPE_VIDEO,
                                                       st_index[AVMEDIA_TYPE_VIDEO], -1, NULL, 0);
    st_index[AVMEDIA_TYPE_AUDIO] = av_find_best_stream(ic, AVMEDIA_TYPE_AUDIO,
                                                       st_index[AVMEDIA_TYPE_AUDIO], st_index[AVMEDIA_TYPE_VIDEO], NULL, 0);

    //打开解码器
    if(st_index[AVMEDIA_TYPE_AUDIO] >= 0){
        stream_component_open(st_index[AVMEDIA_TYPE_AUDIO]);
    }

    ret = -1;
    if(st_index[AVMEDIA_TYPE_VIDEO] >= 0){
        stream_component_open(st_index[AVMEDIA_TYPE_VIDEO]);
    }

    ffp_notify_msg1(this, FFP_MSG_COMPONENT_OPEN);
    std::cout << "read_thread FFP_MSG_COMPONENT_OPEN " << this << std::endl;

    if (video_stream < 0 && audio_stream < 0) {
        av_log(NULL, AV_LOG_FATAL, "Failed to open file '%s' or configure filtergraph\n",
               input_filename_);
        ret = -1;
        goto fail;
    }

    ffp_notify_msg1(this, FFP_MSG_PREPARED);
    std::cout << "read_thread FFP_MSG_PREPARED " << this << std::endl;

    while (1) {
        //        std::cout << "read_thread sleep, mp:" << this << std::endl;
        // 先模拟线程运行
        std::this_thread::sleep_for(std::chrono::milliseconds(1000));
        if(abort_request) {
            break;
        }
    }

    std::cout << __FUNCTION__ << " leave" << std::endl;

    return 0;
fail:
    return -1;
}

int FFPlayer::start_l()
{
    std::cout << __FUNCTION__;
}

int FFPlayer::stop_l()
{
    abort_request = 1;  // 请求退出
    msg_queue_abort(&msg_queue_);  // 禁止再插入消息
}

