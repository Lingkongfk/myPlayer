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


int FFPlayer::prepare_async_l(const char *data_source)
{
    input_filename_ = strdup(data_source);
    int ret = stream_open(data_source);
    return ret;
}

int FFPlayer::stream_open(const char *file_name)
{
    //初始化帧队列，包队列，时钟，创建read_thread

    read_thread_ = new std::thread(&FFPlayer::read_thread, this);
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

