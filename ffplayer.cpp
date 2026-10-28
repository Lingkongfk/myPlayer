#include "ffplayer.h"
#include "ffmsg.h"
#include <string.h>
#include <iostream>
#include <algorithm>
#include <QDebug>
//sdl缓冲区大小
#define SDL_AUDIO_MIN_BUF_SIZE 512

//回调函数每秒读取数据大小
#define SDL_AUDIO_MAX_CALLBACKS_PRE_SEC 30

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

    // 初始化Packet包队列，当前类封装，无需手动init
    // if (packet_queue_init(&videoq) < 0 ||
    //     packet_queue_init(&audioq) < 0 )
    //     goto fail;
    //初始化时钟
    //init_clock(&audclk);


    read_thread_ = new std::thread(&FFPlayer::read_thread, this);

    video_refresh_thread_ = new std::thread(FFPlayer::video_refresh_thread, this);

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
    videoq.~PacketQueue();
    audioq.~PacketQueue();
    // packet_queue_destroy(&videoq);
    // packet_queue_destroy(&audioq);
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
    avctx->pkt_timebase = ic->streams[stream_index]->time_base;
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

        if(audio_open(channel_layout, nb_channels,sample_rate,&audio_tgt) < 0){
            goto fail;
        }
        audio_hw_buf_size = ret;
        audio_src = audio_tgt;  //暂且将数据源参数等同于目标输出参数
        //初始化audio_buf相关参数
        audio_buf_size  = 0;
        audio_buf_index = 0;

        audio_stream = stream_index;
        audio_st = ic->streams[audio_stream];

        //启动解码器线程
        //运行音频输出
        qDebug() << "audio thread start";

        auddec.decoder_init(avctx, &audioq);
        auddec.decoder_start(AVMEDIA_TYPE_AUDIO, "audio_thread", this);
        // 允许音频输出
        //play audio
        SDL_PauseAudio(0);

        break;
    case AVMEDIA_TYPE_VIDEO:
        video_stream = stream_index;
        video_st = ic->streams[video_stream];

        //初始化ffplay的解码器
        //启动解码线程
        qDebug() << "video thread init";
        viddec.decoder_init(avctx, &videoq); //  is->continue_read_thread
        //        // 启动视频频解码线程
        qDebug() << "video thread start";
        if ((ret = viddec.decoder_start(AVMEDIA_TYPE_VIDEO, "video_decoder", this)) < 0)
            goto out;
        qDebug() << "video thread back";
        break;
    default:
        break;
    }
out:
    return ret;

fail:
    avcodec_free_context(&avctx);
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
        auddec.decoder_abort(&sampq);
        // 关闭音频设备
        audio_close();
        // 销毁解码器
        auddec.decoder_destory();
        // 释放重采样器
        swr_free(&swr_ctx);
        // 释放audio buf
        av_freep(&audio_buf1);
        audio_buf1_size = 0;
        audio_buf = NULL;
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
        // 请求终止画面刷新线程
        if(video_refresh_thread_ && video_refresh_thread_->joinable()) {
            video_refresh_thread_->join();  // 等待线程退出
        }

        std::cout << __FUNCTION__ << "  AVMEDIA_TYPE_VIDEO\n";
        // 请求终止解码器线程
        // 关闭音频设备
        // 销毁解码器
        viddec.decoder_abort(&pictq);
        viddec.decoder_destory();

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
    title_duration = ic->duration;
    add_duration_callback_(title_duration);

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
        if(abort_request) {
            break;
        }

        //如果有seek操作
        if(seek_req){
            int64_t seek_max = (seek_rel < 0? seek_pos - seek_rel - 2 : INT_MAX);
            int64_t seek_min = (seek_rel > 0? seek_pos - seek_rel + 2 : INT_MIN);
            int64_t seek_target = seek_pos;

            avformat_seek_file(ic, -1, seek_min, seek_target, seek_max, seek_flag);

            if(audio_stream >= 0){
                audioq.flush();
                AVPacket pkt_flush{};//冲刷解码器
                audioq.push(&pkt_flush);
            }
            if(video_stream >= 0){
                videoq.flush();
                AVPacket pkt_flush{};
                videoq.push(&pkt_flush);
            }
            seek_req = 0;
            eof = 0;
        }

        ret = av_read_frame(ic, pkt);
        if(ret < 0){
            //读完了
            if((ret == AVERROR_EOF ||  avio_feof(ic->pb)) && !eof){
                eof = 1;
            }
            if(ic->pb && ic->pb->error)
                break;

            std::unique_lock<std::mutex> lock(mtx);
            continue_read_thread.wait_for(lock, std::chrono::milliseconds(10));
            continue;
        }else{
            eof = 0;
        }

        if(pkt->stream_index == audio_stream){
            //printf("audio ===== pkt pts:%ld, dts:%ld\n", pkt->pts/48, pkt->dts);
            audioq.push(pkt);
        }else if(pkt->stream_index == video_stream){
            //printf("video ===== pkt pts:%ld, dts:%ld\n", pkt->pts/48, pkt->dts);
            videoq.push(pkt);
        }
        else{
            av_packet_unref(pkt);
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


Decoder::Decoder()
{
    av_init_packet(&pkt_);
}

Decoder::~Decoder()
{

}

void Decoder::decoder_init(AVCodecContext *avctx, PacketQueue *queue)
{
    avctx_ = avctx;
    queue_ = queue;
}

int Decoder::decoder_start(AVMediaType codec_type, const char *thread_name, void *arg)
{
    //packet_queue_start(queue_);
    queue_->start();
    if(AVMEDIA_TYPE_AUDIO == codec_type){
        decoder_thread_ = new std::thread(&Decoder::audio_thread, this, arg);
    }else if(AVMEDIA_TYPE_VIDEO == codec_type){
        decoder_thread_ = new std::thread(&Decoder::video_thread, this, arg);
    }else{
        return -1;
    }
    return 0;
}

void Decoder::decoder_abort(FrameQueue *fq)
{
    queue_->start();
    frame_queue_signal(fq);
    if(decoder_thread_ && decoder_thread_->joinable()){
        decoder_thread_->join();
        delete decoder_thread_;
        decoder_thread_ = NULL;
    }
    queue_->flush();
}

void Decoder::decoder_destory()
{
    av_packet_unref(&pkt_);
    avcodec_free_context(&avctx_);
}

//解码核心
int Decoder::decoder_decode_frame(AVFrame *frame)
{
    int ret = AVERROR(EAGAIN);
    for(;;){
        AVPacket pkt{};
        do{
            if(queue_->getAbort_request()){
                return -1;
            }
            switch(avctx_->codec_type){
            case AVMEDIA_TYPE_AUDIO:
                ret = avcodec_receive_frame(avctx_, frame);
                if(ret >= 0){
                    AVRational tb = {1, frame->sample_rate};
                    if(frame->pts != AV_NOPTS_VALUE){
                        frame->pts = av_rescale_q(frame->pts, avctx_->pkt_timebase, tb);
                    }
                }else{
                    char errStr[256] = { 0 };
                    av_strerror(ret, errStr, sizeof(errStr));
                    //printf("audio dec:%s\n", errStr);
                }
                break;
            case AVMEDIA_TYPE_VIDEO:
                ret = avcodec_receive_frame(avctx_, frame);
                printf("frame pts:%ld, dts:%ld\n", frame->pts, frame->pkt_dts);
                if (ret >= 0) {
                    if (1) {
                       frame->pts = frame->best_effort_timestamp;
                       }
                                       // } else if (!decoder_reorder_pts) {
                                       //     frame->pts = frame->pkt_dts;
                                       // }
                } else {

                    char errStr[256] = { 0 };
                    av_strerror(ret, errStr, sizeof(errStr));
                    //printf("video dec:%s\n", errStr);
                }
                break;
            }


            // 1.3. 检查解码是否已经结束，解码结束返回0
            if (ret == AVERROR_EOF) {
                //printf("avcodec_flush_buffers %s(%d)\n", __FUNCTION__, __LINE__);
                avcodec_flush_buffers(avctx_);
                return 0;
            }
            // 1.4. 正常解码返回1
            if (ret >= 0)
                return 1;       // 获取到一帧frame
        }while(ret != AVERROR(EAGAIN));



        //如果没有能取到的解码frame，就发送
        // if(packet_queue_get(queue_, &pkt, 1, &pkt_serial_) < 0)
        //     return -1;
        if(queue_->pop(&pkt, &pkt_serial_, 1) < 0)
            return -1;

        //qDebug() << "send packet to decode";
        if(avcodec_send_packet(avctx_, &pkt) == AVERROR(EAGAIN)){
            av_log(avctx_, AV_LOG_ERROR, "Receive_frame and send_packet both returned EAGAIN, which is an API violation.\n");
        }
        av_packet_unref(&pkt);	// 一定要自己去释放音视频数据
    }
}

int Decoder::get_video_frame(AVFrame *frame)
{
    int got_picture;
    if((got_picture = decoder_decode_frame(frame)) < 0){
        return -1;
    }
    if(got_picture){

    }
    return got_picture;
}

int Decoder::queue_picture(FrameQueue *fq, AVFrame *src_frame, double pts, double duration, int64_t pos, int serial)
{
    Frame *vp;


    if (!(vp = frame_queue_peek_writable(fq))) // 检测队列是否有可写空间
        return -1;      // 请求退出则返回-1
    // 执行到这步说已经获取到了可写入的Frame
    //    vp->sar = src_frame->sample_aspect_ratio;
    //    vp->uploaded = 0;

    vp->width = src_frame->width;
    vp->height = src_frame->height;
    vp->format = src_frame->format;
    vp->serial = serial;
    vp->pts = pts;
    vp->duration = duration;


    av_frame_move_ref(vp->frame, src_frame); // 将src中所有数据转移到dst中，并复位src。
    frame_queue_push(fq);   // 更新写索引位置
    return 0;
}

int Decoder::audio_thread(void *arg)
{
    std::cout << __FUNCTION__ <<  " into " << std::endl;
    FFPlayer *is = (FFPlayer *)arg;
    AVFrame *frame = av_frame_alloc();  // 分配解码帧
    Frame *af;
    int got_frame = 0;  // 是否读取到帧
    AVRational tb;      // timebase
    int ret = 0;

    if (!frame)
        return AVERROR(ENOMEM);

    do {
        // 1. 读取解码帧
        if ((got_frame = decoder_decode_frame(frame)) < 0)   // 是否获取到一帧数据
            goto the_end; // < =0 abort

        if (got_frame) {
            tb = {1, frame->sample_rate};   // 设置为sample_rate为timebase

            // 2. 获取可写Frame
            if (!(af = frame_queue_peek_writable(&is->sampq)))  // 获取可写帧
                goto the_end;
            // 3. 设置Frame并放入FrameQueue
            af->pts = (frame->pts == AV_NOPTS_VALUE) ? NAN : frame->pts * av_q2d(tb);  // 转换时间戳
            //            af->pos = frame->pkt_pos;
            //            af->serial = is->auddec.pkt_serial;
            af->duration = av_q2d({frame->nb_samples, frame->sample_rate});

            av_frame_move_ref(af->frame, frame);
            frame_queue_push(&is->sampq);  // 代表队列真正插入一帧数据

        }
    } while (ret >= 0 || ret == AVERROR(EAGAIN) || ret == AVERROR_EOF);
the_end:

    std::cout << __FUNCTION__ <<  " leave " << std::endl;
    av_frame_free(&frame);
    return ret;
}

int Decoder::video_thread(void *arg)
{
    std::cout << __FUNCTION__ <<  " into " << std::endl;
    FFPlayer *is = (FFPlayer *)arg;
    AVFrame *frame = av_frame_alloc();  // 分配解码帧
    double pts;                 // pts
    double duration;            // 帧持续时间
    int ret;
    //1 获取stream timebase
    AVRational tb = is->video_st->time_base; // 获取stream timebase
    //2 获取帧率，以便计算每帧picture的duration
    AVRational frame_rate = av_guess_frame_rate(is->ic, is->video_st, NULL);


    if (!frame)
        return AVERROR(ENOMEM);

    for (;;) {  // 循环取出视频解码的帧数据
        // 3 获取解码后的视频帧
        ret = get_video_frame(frame);
        if (ret < 0)
            goto the_end;   //解码结束, 什么时候会结束
        if (!ret)           //没有解码得到画面, 什么情况下会得不到解后的帧
            continue;

        //           1/25 = 0.04秒
        // 4 计算帧持续时间和换算pts值为秒
        // 1/帧率 = duration 单位秒, 没有帧率时则设置为0, 有帧率帧计算出帧间隔
        duration = (frame_rate.num && frame_rate.den ? av_q2d({frame_rate.den, frame_rate.num}) : 0);
        // 根据AVStream timebase计算出pts值, 单位为秒
        pts = (frame->pts == AV_NOPTS_VALUE) ? NAN : frame->pts * av_q2d(tb);  // 单位为秒
        // 5 将解码后的视频帧插入队列
        ret = queue_picture(&is->pictq, frame, pts, duration, frame->pkt_dts, is->viddec.pkt_serial_);
        // 6 释放frame对应的数据
        av_frame_unref(frame);

        if (ret < 0) // 返回值小于0则退出线程
            goto the_end;
    }
the_end:
    std::cout << __FUNCTION__ <<  " leave " << std::endl;
    av_frame_free(&frame);
    return 0;
}

//音频重采样
static int audio_decode_frame(FFPlayer* is){
    if(is->audclk.get_pause()){
        return -1;
    }
    int data_size, resampled_data_size;
    AVChannelLayout dec_channel_layout;
    int wanted_nb_samples;
    Frame* af;
    int ret = 0;

    //判断是否可读，读取一帧
    if(!(af = frame_queue_peek_readable(&is->sampq))){
        return -1;
    }

    //根据frame指定的音频参数获取缓冲区大小
    data_size = av_samples_get_buffer_size(NULL, af->frame->ch_layout.nb_channels
                                           , af->frame->nb_samples
                                           , (AVSampleFormat)af->frame->format
                                           , 1);

    dec_channel_layout = af->frame->ch_layout;
    wanted_nb_samples = af->frame->nb_samples;

    //如果解码出来的数据，和原本的参数不一样，也需要修正
    if(af->frame->format != is->audio_src.fmt ||
        af->frame->ch_layout.nb_channels != is->audio_src.channels.nb_channels ||
        af->frame->sample_rate != is->audio_src.freq){
        swr_free(&is->swr_ctx);
        int ret = swr_alloc_set_opts2(&is->swr_ctx,
                                          &is->audio_tgt.channels,
                                          is->audio_tgt.fmt,
                                          is->audio_tgt.freq,
                                          &dec_channel_layout,
                                          (AVSampleFormat)af->frame->format,
                                          af->frame->sample_rate,
                                          0, NULL);
        if(!is->swr_ctx || swr_init(is->swr_ctx) < 0){
            av_log(NULL, AV_LOG_ERROR,
                   "Cannot create sample rate converter for conversion of %d Hz %s %d channels to %d Hz %s %d channels!\n",
                   af->frame->sample_rate, av_get_sample_fmt_name((AVSampleFormat)af->frame->format), af->frame->ch_layout,
                   is->audio_tgt.freq, av_get_sample_fmt_name((AVSampleFormat)is->audio_tgt.fmt), is->audio_tgt.channels);
            swr_free(&is->swr_ctx);
            ret = -1;
            goto fail;
        }
        is->audio_src.channels = dec_channel_layout;
        is->audio_src.freq = af->frame->sample_rate;
        is->audio_src.fmt = (AVSampleFormat)af->frame->format;

    }

    if(is->swr_ctx){
        const uint8_t **in = (const uint8_t**)af->frame->extended_data;
        uint8_t** out = &is->audio_buf1;
        //计算重采样后的样本数量
        int out_count = (int64_t)wanted_nb_samples * is->audio_tgt.freq / af->frame->sample_rate;
        //计算需要的buffer空间
        int out_size = av_samples_get_buffer_size(NULL, is->audio_tgt.channels.nb_channels
                                                  ,out_count, is->audio_tgt.fmt, 0);
        if(out_size < 0){
            av_log(NULL, AV_LOG_ERROR, "av_samples_get_buffer_size() failed\n");
            ret = -1;
            goto fail;
        }
        //使用out_size给audio_buf1重新分配内存
        av_fast_malloc(&is->audio_buf1, &is->audio_buf1_size, out_size);
        if (!is->audio_buf1) {
            ret = AVERROR(ENOMEM);
            goto fail;
        }

        //启动重采样
        int len2 = swr_convert(is->swr_ctx, out, out_count, in, af->frame->nb_samples);
        if(len2 < 0){
            av_log(NULL, AV_LOG_ERROR, "swr_convert() failed\n");
            ret = -1;
            goto fail;
        }

        if(len2 == out_count){
            //av_log(NULL, AV_LOG_WARNING, "audio buffer is probably too small\n");
            if (swr_init(is->swr_ctx) < 0)
                swr_free(&is->swr_ctx);
        }

        //把重采样后的数据放回audio_buf
        is->audio_buf = is->audio_buf1;
        resampled_data_size = len2* is->audio_tgt.channels.nb_channels * av_get_bytes_per_sample(is->audio_tgt.fmt);
    }else{
        //不进行重采样
        is->audio_buf = af->frame->data[0];
        resampled_data_size = data_size;
    }

    if(!isnan(af->pts)){
        is->audio_clock = af->pts;
    }else{
        is->audio_clock = NAN;
    }

    frame_queue_next(&is->sampq);
    ret = resampled_data_size;


fail:
    return ret;
}

//sdl播放音频时，调用该回调函数读取数据到sdl本身的缓冲区
static void sdl_audio_callback(void* opaque, uint8_t* stream, int len){
    FFPlayer* is = (FFPlayer*)opaque;
    int audio_size, len1;

    double sdl_callback_time = av_gettime_relative() / 1000000.0;
    //本次sdl要读取的数据量
    while(len > 0){
        //缓冲区读完了，继续进行重采样获取数据
        if(is->audio_buf_index >= is->audio_buf_size){
            audio_size = audio_decode_frame(is);
            if(audio_size < 0){
                //这里说明是静音
                is->audio_buf = NULL;
                is->audio_buf_size = SDL_AUDIO_MIN_BUF_SIZE / is->audio_tgt.frame_size * is->audio_tgt.frame_size;

            }else{
                is->audio_buf_size = audio_size;
            }
            is->audio_buf_index = 0;
        }

        //目前缓冲区还有的数据大小
        len1 = is->audio_buf_size - is->audio_buf_index;
        if(len1 > len){
            len1 = len;
        }
        if(is->audio_buf){
            memcpy(stream, (uint8_t*)is->audio_buf + is->audio_buf_index, len1);
        }else{
            memset(stream, 0, len1);
        }

        len -= len1;
        stream += len1;
        is->audio_buf_index += len1;
    }

    if(!isnan(is->audio_clock)){
        int audio_write_buf_size = is->audio_buf_size - is->audio_buf_index;
        is->audclk.set_clock_at(is->audio_clock - (double)(2 * is->audio_hw_buf_size + audio_write_buf_size) / is->audio_tgt.bytes_per_sec,sdl_callback_time, is->audclk.serial);
    }
}


int FFPlayer::audio_open(AVChannelLayout wanted_channel_layout, int wanted_nb_channels, int wanted_sample_rate, AudioParams *audio_hw_params)
{
    SDL_AudioSpec wanted_spec;
    wanted_spec.freq = wanted_sample_rate;
    wanted_spec.format = AUDIO_S16SYS;
    wanted_spec.channels = wanted_nb_channels;
    wanted_spec.silence = 0;
    wanted_spec.samples = 2048;//每次读取的采样数量
    wanted_spec.callback = sdl_audio_callback;
    wanted_spec.userdata = this;

    if(SDL_OpenAudio(&wanted_spec, NULL) != 0){
        printf("Failed to open audio device, %s\n", SDL_GetError());
        return -1;
    }

    audio_hw_params->fmt = AV_SAMPLE_FMT_S16;
    audio_hw_params->freq = wanted_spec.freq;
    audio_hw_params->channels = wanted_channel_layout;
    audio_hw_params->frame_size = av_samples_get_buffer_size(NULL,
                                                             audio_hw_params->channels.nb_channels,
                                                             1,
                                                             audio_hw_params->fmt,
                                                             1);
    audio_hw_params->bytes_per_sec = av_samples_get_buffer_size(NULL, audio_hw_params->channels.nb_channels,
                                                                audio_hw_params->freq,  // 44100
                                                                audio_hw_params->fmt, 1);
    if (audio_hw_params->bytes_per_sec <= 0 || audio_hw_params->frame_size <= 0) {
        av_log(NULL, AV_LOG_ERROR, "av_samples_get_buffer_size failed\n");
        return -1;
    }

    return wanted_spec.size;
}



void FFPlayer::audio_close()
{
    SDL_CloseAudio();
}



void FFPlayer::AddVideoRefreshCallback(std::function<int (const Frame *)> callback)
{
    video_refresh_callback_ = callback;
}

#define REFRESH_RATE 0.01  // 每帧休眠10ms

int FFPlayer::video_refresh_thread()
{
    double remaining_time = 0.0;
    while(!abort_request){
        if(remaining_time > 0.0)
            av_usleep((int)(int64_t)(remaining_time * 1000000.0));
        remaining_time = REFRESH_RATE;
        video_refresh(&remaining_time);
    }
}


double vp_duration(Frame* lastvp, Frame* vp){
    if(lastvp->serial == vp->serial){
        double duration = vp->pts - lastvp->pts;
        if(isnan(duration) || duration < 0 || duration > 0.1){
            return lastvp->duration;
        }else{
            return duration;
        }
    }else{
        return 0.0;
    }
}

double FFPlayer::cal_now_delay(double duration){
    double sync_threshold = 0, diff = 0;

    //获取当前时钟和同步时钟的差值
    diff = vidclk.get_clock() - get_master_clock();

    std::cout << " duration=" << duration << std::endl;
    //我们最精确的差值在 0.04s也就是40ms以内就是同步，最坏100ms以内是同步
    sync_threshold = std::max(0.04, std::min(0.1, duration));
    if(!isnan(diff) && fabs(diff) < 10.0){//这里先限制一下
        if(diff <= -sync_threshold){
            //落后超过阈值，立即播放进行追赶
            duration = std::max(0.0, duration + diff);
        }else if(diff >= sync_threshold && duration > 0.1){
            //视频领先等待音频
            duration = duration + diff;
        }else if(diff >= sync_threshold){
            duration = duration*2;
        }
    }
    std::cout << "diff=" << diff << " delay=" << duration << std::endl;
    return duration;
}

void FFPlayer::stream_seek(int64_t seek_target, long long now)
{
    if(!seek_req){
        seek_pos = seek_target * AV_TIME_BASE;
        seek_rel = (seek_target - now) * AV_TIME_BASE;
        seek_flag &= ~AVSEEK_FLAG_BYTE;
        seek_req = 1;
        continue_read_thread.notify_all();
    }
}

void FFPlayer::video_refresh(double *remaining_time)
{
    Frame *vp = NULL;
    // 目前我们先是只有队列里面有视频帧可以播放，就先播放出来
    // 判断有没有视频画面
    if(video_st) {
        double diff, time;
        double last_duration, duration, delay;
        Frame* vp, *lastvp;
retry:
        if (frame_queue_nb_remaining(&pictq) == 0) {
            // 什么都不用做，可以直接退出了
            return;
        }

        // 能跑到这里说明帧队列不为空，肯定有frame可以读取
        lastvp = frame_queue_peek_last(&pictq);
        vp = frame_queue_peek(&pictq);  // 读取待显示帧

        //qDebug() << "now serial: " << vp->serial;
        //如果seek操作，重新读帧显示
        if(vp->serial != videoq.getSerial()){
            frame_queue_next(&pictq);
            goto retry;
        }

        //seek时更新当前帧显示的时钟
        if(vp->serial != lastvp->serial){
            frame_timer = av_gettime_relative() / 1000000.0;
        }


        //如果暂停，就播放上一帧
        if(pause){
            goto display;
        }

        //获得上一帧的播放时长
        last_duration = vp_duration(lastvp, vp);
        delay = cal_now_delay(last_duration);


        //获得当前系统时间，查看是否需要继续播放一段时间
        time = av_gettime_relative() / 1000000.0;
        if(time < frame_timer + delay){
            *remaining_time = std::min(frame_timer + delay - time, *remaining_time);
            goto display;
        }
        //如果上一帧播放落后或者正常

        //更新frame_timer
        frame_timer += delay;

        //如果frame_timer落后系统时间超过100ms就更新为系统时间，这步在播放第一帧的时候一定会被调用
        if(delay > 0 && time - frame_timer > 0.1){
            frame_timer = time;
        }

        //即将播放，同步当前视频时钟
        SDL_LockMutex(pictq.mutex);
        if(!isnan(vp->pts)){
            vidclk.set_clock(vp->pts, vp->serial);
        }
        SDL_UnlockMutex(pictq.mutex);

        if(frame_queue_nb_remaining(&pictq) > 1){
            Frame* nextvp = frame_queue_peek_next(&pictq);
            duration = vp_duration(vp, nextvp);
            //按照时间下一帧都已经播放了，直接丢帧
            if(time > frame_timer + duration){
                frame_queue_next(&pictq);
                goto retry;
            }
        }

        frame_queue_next(&pictq);

display:
        // 刷新显示
        if(video_refresh_callback_ && pictq.rindex_shown)
            video_refresh_callback_(frame_queue_peek_last(&pictq));
        else
            std::cout << __FUNCTION__ << " video_refresh_callback_ NULL" << std::endl;
    }
}

void FFPlayer::AddDurationCallback(std::function<void (long long)> callback)
{
    add_duration_callback_ = callback;
}

int FFPlayer::get_master_sync_type()
{
    if (av_sync_type == AV_SYNC_VIDEO_MASTER) {
        if (video_st)
            return AV_SYNC_VIDEO_MASTER;
        else
            return AV_SYNC_AUDIO_MASTER;	 /* 如果没有视频成分则使用 audio master */
    } else if (av_sync_type == AV_SYNC_AUDIO_MASTER) {
        if (audio_st)
            return AV_SYNC_AUDIO_MASTER;
        else if(video_st)
            return AV_SYNC_VIDEO_MASTER;        // 只有音频的存在
        else
            return AV_SYNC_UNKNOW_MASTER;
    }
}

double FFPlayer::get_master_clock()
{
    double val;

    switch (get_master_sync_type()) {
    case AV_SYNC_VIDEO_MASTER:
        //        val = get_clock(&vidclk);
        break;
    case AV_SYNC_AUDIO_MASTER:
        val = audclk.get_clock();
        break;
    default:
        val = audclk.get_clock();
        break;
    }
    return val;
}

int FFPlayer::Pause()
{
    if(this->pause){
        //当前是暂停状态，要继续播放
        frame_timer += (av_gettime_relative() / 1000000.0) - vidclk.last_updated;
        vidclk.set_clock(vidclk.get_clock(), vidclk.serial);
        audclk.set_clock(audclk.get_clock(), audclk.serial);
    }else{
        //当前正在播放，要进行暂停

    }
    this->pause = !this->pause;
    audclk.set_pause_val(this->pause);
    vidclk.set_pause_val(this->pause);
}