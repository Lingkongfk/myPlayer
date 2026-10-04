#include "ffplayer.h"
#include "ffmsg.h"
#include <string.h>

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

int FFPlayer::stream_component_open(int stream_index)
{

}

void FFPlayer::stream_component_close(int stream_index)
{

}

//这里模拟一下运行
int FFPlayer::read_thread()
{
    ffp_notify_msg1(this, FFP_MSG_OPEN_INPUT);
    std::cout << "read_thread FFP_MSG_OPEN_INPUT " << this << std::endl;
    ffp_notify_msg1(this, FFP_MSG_FIND_STREAM_INFO);
    std::cout << "read_thread FFP_MSG_FIND_STREAM_INFO " << this << std::endl;
    ffp_notify_msg1(this, FFP_MSG_COMPONENT_OPEN);
    std::cout << "read_thread FFP_MSG_COMPONENT_OPEN " << this << std::endl;
    ffp_notify_msg1(this, FFP_MSG_PREPARED);
    std::cout << "read_thread FFP_MSG_PREPARED " << this << std::endl;
    while (1) {
        //        std::cout << "read_thread sleep, mp:" << this << std::endl;
        // 先模拟线程运行
        std::this_thread::sleep_for(std::chrono::milliseconds(1000));
    }
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

