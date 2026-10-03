#ifndef MYMEDIAPLAYER_H
#define MYMEDIAPLAYER_H

#include <mutex>
#include <thread>
#include <functional>
#include "ffmsg_queue.h"
#include "ffmsg.h"
#include "ffplayer.h"

class MyMediaPlayer
{
public:
    MyMediaPlayer();
    ~MyMediaPlayer();

    //这些公有接口都是供UI进行调用的
    //播放器创建
    int create(std::function<int(void*)> msg_loop);
    //销毁播放器
    int destory();
    //异步准备
    int prepare_async();
    //启动播放器
    int start();
    //停止播放器
    int stop();
    //暂停
    int pause();
    //seek到指定位置
    int seek_to(long long msec);
    //获取播放的状态
    int get_state();
    //判断是否正在播放
    bool is_playing();
    //获取当前的播放位置
    long long get_current_position();
    //获取视频总时长
    long long get_duration();
    //获取已经播放的长度
    long long get_playable_duration();
    //设置是否循环播放
    void set_loop();
    //获取是否循环播放
    int get_loop();
    //读取循环消息
    int get_msg(AVMessage* msg, int block);
    // 设置音量
    void set_playback_volume(float volume);
    //设置媒体源
    int set_data_source(const char* url);

    //调用UI处理循环函数的函数，该函数本身会是一个线程
    int msg_loop(void* arg);

private:
    std::mutex mutex_;         //线程安全互斥量
    FFPlayer* ffplayer_ = NULL;//ffplayer的指针
    std::function<int(void*)> msg_loop_ = NULL; //ui处理循环函数（ffplayer发来的）
    std::thread* msg_thread_;                   //执行msg_loop_的线程
    char* data_source_;        //媒体源的url或者文件名
    MEDIA_STATE state_;
};

#endif // MYMEDIAPLAYER_H
