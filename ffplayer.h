#ifndef FFPLAYER_H
#define FFPLAYER_H

#include <ffmsg_queue.h>
#include <thread>
#include "ffplay_def.h"

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

    int audio_stream = -1;
    int video_stream = -1;
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
