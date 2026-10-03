#include "ffmsg_queue.h"
#include "ffmsg.h"
extern "C"{
#include <libavformat/avformat.h>
#include <libavformat/avio.h>
#include <libavformat/version.h>
#include <libavformat/version_major.h>
#include <libavcodec/avcodec.h>
#include <libavutil/frame.h>
#include <libavutil/imgutils.h>
#include <libavutil/opt.h>
#include <libavutil/samplefmt.h>
}

void msg_free_res(AVMessage *msg)
{
    if(!msg || !msg->obj){
        return ;
    }
    //释放自己申请的内存
    msg->free_l(msg->obj);
    msg->obj = NULL;
}

//消息队列内部重新构建一个AVMessage, 重新申请内存(可能是回收的重用)创建
int msg_queue_put_private(MessageQueue *q, AVMessage *msg)
{
    AVMessage* msg1;

    //队列终止时，退出
    if(q->abort_request){
        return -1;
    }

    //从回收队列里面取消息
    msg1 = q->recycle_msg;
    if(msg1){
        q->recycle_msg = msg1->next;
        q->recycle_count++;
    }else{
        //没有能使用的回收消息
        q->alloc_count++;
        msg1 = (AVMessage*)av_malloc(sizeof(AVMessage));
    }
    *msg1 = *msg;//浅拷贝
    msg1->next = NULL;

    //插入队列
    if(!q->first_msg){
        //如果是消息队列的第一个元素
        q->first_msg = msg1;
    }else{
        q->last_msg->next = msg1;
    }
    q->last_msg = msg1;
    q->nb_messages++;
    //队列有元素了，通知条件变量
    q->cond.notify_one();
}

//q->mutex.lock();
//q->mutex.unlock();
//对外开放的放入队列操作，线程安全
int msg_queue_put(MessageQueue *q, AVMessage *msg)
{
    int ret;
    q->mutex.lock();
    ret = msg_queue_put_private(q, msg);
    q->mutex.unlock();
    return ret;
}

//从头部first_msg取消息
int msg_queue_get(MessageQueue *q, AVMessage *msg, int block)
{
    AVMessage* msg1;
    int ret;
    std::unique_lock<std::mutex> lock(q->mutex);
    for(;;){
        if(q->abort_request){
            ret = -1;
            break;
        }
        //获取消息
        msg1 = q->first_msg;
        if(msg1){
            q->first_msg = msg1->next;
            if(!q->first_msg){
                //如果取出一个没有元素了
                q->last_msg = NULL;
            }
            q->nb_messages--;
            *msg = *msg1;//浅拷贝
            msg1->obj = NULL;
            //把msg1放到回收里面
            msg1->next = q->recycle_msg;
            q->recycle_msg = msg1;
            break;
        }else if(!block){
            ret = 0;
            break;
        }else{
            //阻塞方式取消息
            q->cond.wait(lock);
        }
    }

    return ret;
}


//初始化msg
void msg_init_msg(AVMessage *msg)
{
    memset(msg, 0, sizeof(AVMessage));
}


void msg_queue_put_simple1(MessageQueue *q, int what)
{
    //为什么使用拷贝放入队列，这里就体现了，在栈上声明的变量，出函数直接销毁了，所以拷贝过来使用
    AVMessage msg;
    msg_init_msg(&msg);
    msg.what = what;
    msg_queue_put(q, &msg);
}

void msg_queue_put_simple2(MessageQueue *q, int what, int arg1)
{
    AVMessage msg;
    msg_init_msg(&msg);
    msg.what = what;
    msg.arg1 = arg1;
    msg_queue_put(q, &msg);
}

void msg_queue_put_simple3(MessageQueue *q, int what, int arg1, int arg2)
{
    AVMessage msg;
    msg_init_msg(&msg);
    msg.what = what;
    msg.arg1 = arg1;
    msg.arg2 = arg2;
    msg_queue_put(q, &msg);
}

void msg_obj_free_l(void *obj)
{
    av_free(obj);
}

void msg_queue_put_simple4(MessageQueue *q, int what, int arg1, int arg2, void *obj, int obj_len)
{
    AVMessage msg;
    msg_init_msg(&msg);
    msg.what = what;
    msg.arg1 = arg1;
    msg.arg2 = arg2;
    msg.obj = av_malloc(obj_len);
    memcpy(msg.obj, obj, obj_len);
    msg.free_l = msg_obj_free_l;
    msg_queue_put(q, &msg);
}

void msg_queue_init(MessageQueue *q)
{
    memset(q, 0 ,sizeof(MessageQueue));
    q->abort_request = 1;
}

//把消息队列所有消息移动到回收里面
void msg_queue_flush(MessageQueue *q)
{
    AVMessage* msg, *msg1;
    q->mutex.lock();
    for(msg = q->first_msg;msg != NULL;msg = msg1){
        msg1 = msg->next;
        msg->next = q->recycle_msg;
        q->recycle_msg = msg;
    }
    q->last_msg = NULL;
    q->first_msg = NULL;
    q->nb_messages = 0;
    q->mutex.unlock();
}

void msg_queue_destroy(MessageQueue *q)
{
    msg_queue_flush(q);
    q->mutex.lock();
    while(q->recycle_msg){
        AVMessage* msg = q->recycle_msg;
        if(msg){
            q->recycle_msg = msg->next;
        }
        msg_free_res(msg);
        av_freep(&msg);
    }
    q->mutex.unlock();
}

void msg_queue_abort(MessageQueue *q)
{
    q->mutex.lock();
    q->abort_request = 1;
    q->cond.notify_one();
    q->mutex.unlock();
}

void msg_queue_start(MessageQueue *q)
{
    q->mutex.lock();
    q->abort_request = 0;
    //插入一个消息
    AVMessage msg;
    msg_init_msg(&msg);
    msg.what = FFP_MSG_FLUSH;
    msg_queue_put_private(q, &msg);
    q->mutex.unlock();
}

void msg_queue_remove(MessageQueue *q, int what)
{
    AVMessage** p_msg, *msg, *last_msg;
    q->mutex.lock();
    last_msg = q->first_msg;

    if(!q->abort_request && q->first_msg){
        p_msg = &q->first_msg;
        while(*p_msg){
            msg = *p_msg;
            if(msg->what == what){
                *p_msg = msg->next;
                msg_free_res(msg);
                msg->next = q->recycle_msg;
                q->recycle_msg = msg;
                q->nb_messages--;
            }else{
                last_msg = msg;
                p_msg = &msg->next;
            }
        }

        if(q->first_msg){
            q->last_msg = last_msg;
        }else{
            q->last_msg = NULL;
        }
    }

    q->mutex.unlock();
}























