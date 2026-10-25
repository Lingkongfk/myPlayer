#ifndef FFPLAYER_H
#define FFPLAYER_H

#include <ffmsg_queue.h>
#include <thread>
#include "ffplay_def.h"
#include <functional>


//解码器
class Decoder{
public:
    AVPacket pkt_;
    PacketQueue *queue_;
    AVCodecContext* avctx_;
    int pkt_serial_;
    int finished_;
    std::thread *decoder_thread_ = NULL;

    Decoder();
    ~Decoder();
    void decoder_init(AVCodecContext* avctx, PacketQueue* queue);
    int decoder_start(enum AVMediaType codec_type, const char* thread_name, void* arg);
    void decoder_abort(FrameQueue* fq);
    void decoder_destory();
    //解码的核心
    int decoder_decode_frame(AVFrame* frame);
    int get_video_frame(AVFrame* frame);
    int queue_picture(FrameQueue* fq, AVFrame* src_frame, double pts,
                      double duration, int64_t pos, int serial);
    //音频解码函数
    int audio_thread(void* arg);
    //视频解码函数
    int video_thread(void* arg);



};

class FFPlayer
{
public:
    FFPlayer();

    //播放器创建
    int create();
    void destory();
    //异步准备
    int prepare_async_l(const char* data_source);
    //打开stream，初始化一系列队列和时钟
    int stream_open(const char* file_name);
    void stream_close();
    //打开指定stream的解码器，创建解码线程，初始化对应输出
    int stream_component_open(int stream_index);
    //关闭指定流的解码线程
    void stream_component_close(int stream_index);

    //音频函数
    int audio_open(AVChannelLayout wanted_channel_layout, int wanted_nb_channels, int wanted_sample_rate, struct AudioParams* audio_hw_params);
    //关闭音频函数
    void audio_close();

    //视频刷新线程
    std::thread* video_refresh_thread_ = NULL;
    std::function<int(const Frame*)> video_refresh_callback_ = NULL;
    void AddVideoRefreshCallback(std::function<int(const Frame*)> callback);
    int video_refresh_thread();
    void video_refresh(double *remaining_time);


    //读取线程
    int read_thread();

    //播放控制
    int start_l();
    int stop_l();

    std::thread* read_thread_;
    char* input_filename_;
    MessageQueue msg_queue_;

    //帧队列
    FrameQueue pictq;  //视频帧队列
    FrameQueue sampq;  //采样帧队列

    PacketQueue audioq; //音频包队列
    PacketQueue videoq; //视频包队列
    int abort_request = 0;

    AVStream* audio_st = NULL;//音频流
    AVStream* video_st = NULL;//视频流
    int audio_stream = -1;
    int video_stream = -1;

    Decoder auddec; //音频解码器
    Decoder viddec; //视频解码器

    int eof = 0;
    AVFormatContext* ic = NULL;

    //音频相关成员
    struct AudioParams audio_src;//解码后的frame参数
    struct AudioParams audio_tgt;//SDL支持的音频参数
    struct SwrContext* swr_ctx = nullptr;//音频重采样context
    int audio_hw_buf_size = 0; //SDL缓冲区大小
    uint8_t *audio_buf = NULL; //指向需要重采样的数据
    uint8_t *audio_buf1 = NULL;//指向重采样后的数据
    unsigned int audio_buf_size = 0;
    unsigned int audio_buf1_size = 0;
    int audio_buf_index = 0; //更新拷贝位置

    int get_master_sync_type();
    double get_master_clock();
    int av_sync_type = AV_SYNC_AUDIO_MASTER;
    Clock audclk;
    double audio_clock = 0;//当前pts + duration

    std::atomic<int> pause = 0;

    //暂停与继续播放
    int Pause();
};

//内联函数，封装消息队列的几种插入消息方法
inline static void ffp_notify_msg1(FFPlayer *ffp, int what) {
    msg_queue_put_simple3(&ffp->msg_queue_, what, 0, 0);
}

inline static void ffp_notify_msg2(FFPlayer *ffp, int what, int arg1) {
    msg_queue_put_simple3(&ffp->msg_queue_, what, arg1, 0);
}

inline static void ffp_notify_msg3(FFPlayer *ffp, int what, int arg1, int arg2) {
    msg_queue_put_simple3(&ffp->msg_queue_, what, arg1, arg2);
}

inline static void ffp_notify_msg4(FFPlayer *ffp, int what, int arg1, int arg2, void *obj, int obj_len) {
    msg_queue_put_simple4(&ffp->msg_queue_, what, arg1, arg2, obj, obj_len);
}

inline static void ffp_remove_msg(FFPlayer *ffp, int what) {
    msg_queue_remove(&ffp->msg_queue_, what);
}

#endif // FFPLAYER_H
