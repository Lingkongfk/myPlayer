#include "ffplay_def.h"


PacketQueue::PacketQueue()
{
    first = last = nullptr;
    size = 0;
    nb_packets = 0;
    duration = 0;
    serial = 0;
    abort_request = 1;
}

PacketQueue::~PacketQueue()
{
    abort();
    flush();
}

int PacketQueue::start()
{
    std::unique_lock<std::mutex> lock(mtx);
    serial++;
    abort_request = 0;
}

int PacketQueue::push(AVPacket *pkt)
{
    AVPacket* newpkt = av_packet_alloc();
    if(!newpkt){
        fprintf(stderr, "packetqueue av_packet_alloc failed\n");
        return -1;
    }
    av_packet_move_ref(newpkt, pkt);

    //加锁保证线程安全
    std::unique_lock<std::mutex> lock(mtx);
    int ret = push_private(newpkt);

    if(ret != 0){
        av_packet_free(&newpkt);
        return ret;
    }

    return 0;
}

int PacketQueue::pop(AVPacket *pkt, int* get_serial, int block)
{
    int ret = 0;
    std::unique_lock<std::mutex> lock(mtx);
    while(true){
        if(abort_request){
            ret = -1;
            break;
        }

        //队列内有元素
        if(first){
            PacketNode* firstnode = first;

            if(first == last){
                first = nullptr;
                last = nullptr;
            }else{
                first = first->next;
            }
            size--;
            duration -= firstnode->pkt->duration;
            nb_packets--;
            *get_serial = firstnode->serial;
            av_packet_move_ref(pkt, firstnode->pkt);
            av_packet_free(&firstnode->pkt);
            delete firstnode;

            break;
        }else{
            //队列内无元素
            if(block){
                //等待队列内有元素，或者队列终止
                cond.wait(lock, [=](){ return size > 0 || abort_request;});
            }else{
                ret = -1;
                break;
            }
        }
    }

    return ret;
}

void PacketQueue::abort()
{
    std::unique_lock<std::mutex> lock(mtx);
    abort_request = 1;
    cond.notify_all();
}

void PacketQueue::flush()
{
    std::unique_lock<std::mutex> lock(mtx);
    PacketNode* tmp = first;
    while(tmp){
        PacketNode* nextpkt = tmp->next;
        av_packet_free(&tmp->pkt);
        delete tmp;
        tmp = nextpkt;
    }
    size = 0;
    serial++;
    duration = 0;
    nb_packets = 0;
    first = last = nullptr;
}

//把包放入队列
int PacketQueue::push_private(AVPacket *pkt)
{
    if(pkt == nullptr){
        return -1;
    }
    if(abort_request){
        return -1;
    }

    PacketNode* newNode = new PacketNode;

    newNode->pkt = pkt;
    newNode->next = nullptr;
    newNode->serial = serial;

    //队列无元素
    if(first == nullptr){
        first = newNode;
        last = newNode;
    }else{
        //队列有元素，插入到last后面
        last->next = newNode;
        last = newNode;
    }
    size++;
    nb_packets++;
    duration += pkt->duration;
    //通知条件变量，有元素了
    cond.notify_all();
    return 0;
}



static void frame_queue_unref_item(Frame *vp)
{
    av_frame_unref(vp->frame);	/* 释放数据 */
}

/* 初始化FrameQueue，视频和音频keep_last设置为1，字幕设置为0 */
int frame_queue_init(FrameQueue *f, PacketQueue *pktq, int max_size, int keep)
{
    int i;
    memset(f, 0, sizeof(FrameQueue));
    if (!(f->mutex = SDL_CreateMutex())) {
        av_log(NULL, AV_LOG_FATAL, "SDL_CreateMutex(): %s\n", SDL_GetError());
        return AVERROR(ENOMEM);
    }
    if (!(f->cond = SDL_CreateCond())) {
        av_log(NULL, AV_LOG_FATAL, "SDL_CreateCond(): %s\n", SDL_GetError());
        return AVERROR(ENOMEM);
    }
    f->pktq = pktq;
    f->max_size = FFMIN(max_size, FRAME_QUEUE_SIZE);
    f->keep_last = !!keep;
    for (i = 0; i < f->max_size; i++)
        if (!(f->queue[i].frame = av_frame_alloc())) // 分配AVFrame结构体
            return AVERROR(ENOMEM);
    return 0;
}

void frame_queue_destory(FrameQueue *f)
{
    int i;
    for (i = 0; i < f->max_size; i++) {
        Frame *vp = &f->queue[i];
        // 释放对vp->frame中的数据缓冲区的引用，注意不是释放frame对象本身
        frame_queue_unref_item(vp);
        // 释放vp->frame对象
        av_frame_free(&vp->frame);
    }
    SDL_DestroyMutex(f->mutex);
    SDL_DestroyCond(f->cond);
}

void frame_queue_signal(FrameQueue *f)
{
    SDL_LockMutex(f->mutex);
    SDL_CondSignal(f->cond);
    SDL_UnlockMutex(f->mutex);
}

/* 获取队列当前Frame, 在调用该函数前先调用frame_queue_nb_remaining确保有frame可读 */
Frame *frame_queue_peek(FrameQueue *f)
{
    return &f->queue[(f->rindex + f->rindex_shown) % f->max_size];
}

/* 获取当前Frame的下一Frame, 此时要确保queue里面至少有2个Frame */
// 不管你什么时候调用，返回来肯定不是 NULL
Frame *frame_queue_peek_next(FrameQueue *f)
{
    return &f->queue[(f->rindex + f->rindex_shown + 1) % f->max_size];
}

/* 获取last Frame：
 */
Frame *frame_queue_peek_last(FrameQueue *f)
{
    return &f->queue[f->rindex];
}
// 获取可写指针
Frame *frame_queue_peek_writable(FrameQueue *f)
{
    /* wait until we have space to put a new frame */
    SDL_LockMutex(f->mutex);
    while (f->size >= f->max_size &&
           !f->pktq->getAbort_request()) {	/* 检查是否需要退出 */
        SDL_CondWait(f->cond, f->mutex);
    }
    SDL_UnlockMutex(f->mutex);

    if (f->pktq->getAbort_request())			 /* 检查是不是要退出 */
        return NULL;

    return &f->queue[f->windex];
}
// 获取可读
Frame *frame_queue_peek_readable(FrameQueue *f)
{
    /* wait until we have a readable a new frame */
    SDL_LockMutex(f->mutex);
    while (f->size - f->rindex_shown <= 0 &&
           !f->pktq->getAbort_request()) {
        SDL_CondWait(f->cond, f->mutex);
    }
    SDL_UnlockMutex(f->mutex);

    if (f->pktq->getAbort_request())
        return NULL;

    return &f->queue[(f->rindex + f->rindex_shown) % f->max_size];
}
// 更新写指针
void frame_queue_push(FrameQueue *f)
{
    if (++f->windex == f->max_size)
        f->windex = 0;
    SDL_LockMutex(f->mutex);
    f->size++;
    SDL_CondSignal(f->cond);    // 当_readable在等待时则可以唤醒
    SDL_UnlockMutex(f->mutex);
}

/* 释放当前frame，并更新读索引rindex */
void frame_queue_next(FrameQueue *f)
{
    if(f->keep_last && !f->rindex_shown){
        f->rindex_shown = 1;
        return ;
    }
    frame_queue_unref_item(&f->queue[f->rindex]);
    if (++f->rindex == f->max_size)
        f->rindex = 0;
    SDL_LockMutex(f->mutex);
    f->size--;
    SDL_CondSignal(f->cond);
    SDL_UnlockMutex(f->mutex);
}

/* return the number of undisplayed frames in the queue */
int frame_queue_nb_remaining(FrameQueue *f)
{
    return f->size - f->rindex_shown;
}

/* return last shown position */
int64_t frame_queue_last_pos(FrameQueue *f)
{
    //Frame *fp = &f->queue[f->rindex];
    //if (f->rindex_shown && fp->serial == f->pktq->serial)
    //    return fp->pos;
    //else
    return -1;
}


Clock::Clock()
{
    pause = 0;
    serial = 0;
    set_clock(NAN);
}

double Clock::get_clock()
{
    if(this->pause){
        return this->pts;
    }
    double time = av_gettime_relative() / 1000000.0;
    return time + pts_drift;
}

void Clock::set_clock_at(double pts, double time)
{
    this->pts = pts;
    this->last_updated = time;
    this->pts_drift = pts - time;
}

void Clock::set_clock(double pts)
{
    double time = av_gettime_relative() / 1000000.0;
    set_clock_at(pts, time);
}
