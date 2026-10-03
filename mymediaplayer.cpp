#include "mymediaplayer.h"
#include <iostream>
#include <string.h>

MyMediaPlayer::MyMediaPlayer() {
    std::cout << " IjkMediaPlayer()\n ";
}

MyMediaPlayer::~MyMediaPlayer()
{
    std::cout << " ~IjkMediaPlayer()\n ";
}


/**
 * @brief MyMediaPlayer::create 创建播放器
 * @param msg_loop 传入的线程循环函数
 * 创建播放器myMediaPlayer
 * 创建FFPlayer
 * 保存ui传来的msg_loop
 * 初始化mutex
 */
int MyMediaPlayer::create(std::function<int(void*)> msg_loop)
{
    int ret = 0;
    ffplayer_ = new FFPlayer();
    if(!ffplayer_){
        std::cout << "new FFPlayer failed\n" << std::endl;
        return -1;
    }

    msg_loop_ = msg_loop;

    ret = ffplayer_->create();
    if(ret < 0){
        ret = -1;
    }
    return ret;
}

int MyMediaPlayer::destory()
{
    return 0;
}

/**
 * @brief MyMediaPlayer::prepare_async
 * 把状态设置为正在准备
 * 启动消息队列msg_queue
 * 创建msg_loop线程
 * 调用FFPlayer的prepare_async_l 内部调用stream open 保存filename
 */
int MyMediaPlayer::prepare_async()
{
    state_ = MEDIA_STATE::PREPARING;

    msg_queue_start(&ffplayer_->msg_queue_);

    msg_thread_ =new std::thread(&MyMediaPlayer::msg_loop, this, this);

    int ret = ffplayer_->prepare_async_l(data_source_);
    if(ret < 0){
        state_ = MEDIA_STATE::ERROR;
        return -1;
    }
    return 0;
}

int MyMediaPlayer::start()
{
    ffp_notify_msg1(ffplayer_, FFP_REQ_START);
}

int MyMediaPlayer::get_msg(AVMessage *msg, int block)
{
    while(true){
        int continue_wait_next_msg = 0;
        int ret = msg_queue_get(&ffplayer_->msg_queue_, msg, block);
        if(ret <= 0){
            return ret;
        }
        switch(msg->what){
        case FFP_MSG_PREPARED:
            std::cout << __FUNCTION__ << " FFP_MSG_PREPARED" << std::endl;
            break;
        case FFP_REQ_START:
            std::cout << __FUNCTION__ << " FFP_REQ_START" << std::endl;
            continue_wait_next_msg = 1;
            break;
        default:
            std::cout << __FUNCTION__ << " default " << msg->what << std::endl;
            break;
        }
        if(continue_wait_next_msg){
            msg_free_res(msg);
            continue;
        }
        return ret;
    }
    return -1;
}

int MyMediaPlayer::set_data_source(const char *url)
{
    if(!url){
        return -1;
    }
    data_source_ = strdup(url);
    return 0;
}

int MyMediaPlayer::msg_loop(void *arg)
{
    msg_loop_(arg);
    return 0;
}
