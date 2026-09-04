#ifndef FFMSG_QUEUE_H
#define FFMSG_QUEUE_H

#include <iostream>
#include <mutex>
#include <condition_variable>

typedef struct AVMessage{
    int what;                 //消息类型
    int arg1;                 //参数2
    int arg2;                 //参数1
    void* obj;                //额外参数，malloc分配
    void (*free_l)(void* obj);//释放obj的函数
    struct AVMessage* next;   //下一个消息
}AVMessage;


typedef struct MessageQueue{
    AVMessage* first_msg, *last_msg; //消息队列头部，尾部
    int nb_messages;                //消息个数
    int abort_request;              //请求终止消息队列
    std::mutex mutex;               //互斥锁
    std::condition_variable cond;   //条件变量
    AVMessage* recycle_msg;         //消息循环使用
    int recycle_count;              //循环次数，利用局部性
    int alloc_count;                //分配次数

}MessageQueue;

//释放msg的obj资源
void msg_free_res(AVMessage *msg);

//私有插⼊消息
int msg_queue_put_private(MessageQueue  *q,AVMessage  *msg);

//插⼊消息
int msg_queue_put(MessageQueue *q,AVMessage *msg);

//初始化消息
void msg_init_msg(AVMessage *msg);

//插⼊简单消息，只带消息类型，不带参数
void msg_queue_put_simple1(MessageQueue *q, int what);

//插⼊简单消息，只带消息类型，只带1个参数
void msg_queue_put_simple2(MessageQueue *q, int what, int arg1);

//插⼊简单消息，只带消息类型，带2个参数
void msg_queue_put_simple3(MessageQueue *q, int what, int arg1, int arg2);

//释放msg的obj资源
void msg_obj_free_l(void *obj);


//插⼊消息，带消息类型，带2个参数，带obj
void msg_queue_put_simple4(MessageQueue *q,int what, int arg1, int arg2, void *obj, int obj_len);

//消息队列初始化
void msg_queue_init(MessageQueue *q);

//消息队列flush，清空所有的消息
void msg_queue_flush(MessageQueue *q);

//消息销毁
void msg_queue_destroy(MessageQueue  *q);

//消息队列终⽌
void msg_queue_abort(MessageQueue *q);

//启⽤消息队列
void msg_queue_start(MessageQueue  *q);

//读取消息 if < 0 abort  if >= 0 has msg
int msg_queue_get(MessageQueue  *q,  AVMessage  *msg,int  block);

//消息删除  把队列⾥同⼀消息类型的消息全删除掉
void msg_queue_remove(MessageQueue *q, int what);








#endif // FFMSG_QUEUE_H
