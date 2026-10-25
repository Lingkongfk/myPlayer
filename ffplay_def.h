#ifndef FFPLAY_DEF_H
#define FFPLAY_DEF_H

#include <mutex>
#include <condition_variable>
#include <memory>
#include <functional>
#include <atomic>

#include <inttypes.h>
#include <math.h>
#include <limits.h>
#include <signal.h>
#include <stdint.h>

extern "C" {
#include "libavutil/avstring.h"
#include "libavutil/eval.h"
#include "libavutil/mathematics.h"
#include "libavutil/pixdesc.h"
#include "libavutil/imgutils.h"
#include "libavutil/dict.h"
#include "libavutil/parseutils.h"
#include "libavutil/samplefmt.h"
#include "libavutil/avassert.h"
#include "libavutil/time.h"
#include "libavformat/avformat.h"
#include "libavdevice/avdevice.h"
#include "libswscale/swscale.h"
#include "libavutil/opt.h"
#include <libavcodec/avcodec.h>
#include "libswresample/swresample.h"
#include <SDL.h>
#include <SDL_thread.h>
#undef main
}


#include <assert.h>

//返回值对应的错误码
enum RET_CODE
{
    RET_ERR_UNKNOWN = -2,                   // 未知错误
    RET_FAIL = -1,							// 失败
    RET_OK	= 0,							// 正常
    RET_ERR_OPEN_FILE,						// 打开文件失败
    RET_ERR_NOT_SUPPORT,					// 不支持
    RET_ERR_OUTOFMEMORY,					// 没有内存
    RET_ERR_STACKOVERFLOW,					// 溢出
    RET_ERR_NULLREFERENCE,					// 空参考
    RET_ERR_ARGUMENTOUTOFRANGE,				//
    RET_ERR_PARAMISMATCH,					//
    RET_ERR_MISMATCH_CODE,                  // 没有匹配的编解码器
    RET_ERR_EAGAIN,
    RET_ERR_EOF
};

typedef struct AudioParams{
    int freq;//采样率
    AVChannelLayout channels; //通道布局
    enum AVSampleFormat fmt;//采样格式
    int frame_size; //一个采样的大小
    int bytes_per_sec; //每秒字节数
} AudioParams;


//包节点，我们的包队列使用链表实现
struct PacketNode{
    AVPacket* pkt;    //包
    int serial;       //播放序列
    PacketNode* next; //下一个节点
};

class PacketQueue{
public:
    PacketQueue();
    ~PacketQueue();

    //启动
    int start();
    //放入队列，直接把参数的pkt移动走，外部传入的pkt可以直接free
    int push(AVPacket* pkt);
    //获得并且出队列，需要外部申请pkt的内存，直接把队列的数据移动到外部，队列内部直接销毁
    int pop(AVPacket* pkt, int* get_serial, int block = 1);
    //终止队列
    void abort();
    //刷新队列
    void flush();

    int getSerial(){
        std::unique_lock<std::mutex> lock(mtx);
        return serial;
    }
    int getSize(){
        std::unique_lock<std::mutex> lock(mtx);
        return size;
    }
    int getAbort_request(){
        std::unique_lock<std::mutex> lock(mtx);
        return abort_request;
    }
private:
    int push_private(AVPacket* pkt);

    PacketNode* first, *last; //首尾指针
    int size;  //队列大小
    int nb_packets; //包数量
    double duration;//队列总时长
    int serial; //队列当前播放序列
    int abort_request;//终止标志
    std::mutex mtx;//锁
    std::condition_variable cond;//条件变量
};



#define VIDEO_PICTURE_QUEUE_SIZE	3       // 图像帧缓存数量
#define VIDEO_PICTURE_QUEUE_SIZE_MIN        (3)
#define VIDEO_PICTURE_QUEUE_SIZE_MAX        (16)
#define VIDEO_PICTURE_QUEUE_SIZE_DEFAULT    (VIDEO_PICTURE_QUEUE_SIZE_MIN)
#define SUBPICTURE_QUEUE_SIZE		16      // 字幕帧缓存数量
#define SAMPLE_QUEUE_SIZE           9       // 采样帧缓存数量
#define FRAME_QUEUE_SIZE FFMAX(SAMPLE_QUEUE_SIZE, FFMAX(VIDEO_PICTURE_QUEUE_SIZE, SUBPICTURE_QUEUE_SIZE))


// 用于缓存解码后的数据
typedef struct Frame {
    AVFrame		*frame;         // 指向数据帧
    double		pts;            // 时间戳，单位为秒
    double		duration;       // 该帧持续时间，单位为秒
    int		width;              // 图像宽度
    int		height;             // 图像高读
    int		format;             // 对于图像为(enum AVPixelFormat)
} Frame;

/* 这是一个循环队列，windex是指其中的首元素，rindex是指其中的尾部元素. */
typedef struct FrameQueue {
    Frame	queue[FRAME_QUEUE_SIZE];        // FRAME_QUEUE_SIZE  最大size, 数字太大时会占用大量的内存，需要注意该值的设置
    int		rindex;                         // 读索引。待播放时读取此帧进行播放，播放后此帧成为上一帧
    int		windex;                         // 写索引
    int		size;                           // 当前总帧数
    int		max_size;                       // 可存储最大帧数
    SDL_mutex	*mutex;                     // 互斥量
    SDL_cond	*cond;                      // 条件变量
    PacketQueue	*pktq;                      // 数据包缓冲队列
    int     keep_last;                      //是否保留一个帧
    int     rindex_shown = 0;
} FrameQueue;


/* 初始化FrameQueue，视频和音频keep_last设置为1，字幕设置为0 */
int frame_queue_init(FrameQueue *f, PacketQueue *pktq, int max_size, int keep = 1);
void frame_queue_destory(FrameQueue *f);
void frame_queue_signal(FrameQueue *f);
/* 获取队列当前Frame, 在调用该函数前先调用frame_queue_nb_remaining确保有frame可读 */
Frame *frame_queue_peek(FrameQueue *f);

/* 获取当前Frame的下一Frame, 此时要确保queue里面至少有2个Frame */
// 不管你什么时候调用，返回来肯定不是 NULL
Frame *frame_queue_peek_next(FrameQueue *f);
/* 获取last Frame：
 */
Frame *frame_queue_peek_last(FrameQueue *f);
// 获取可写指针
Frame *frame_queue_peek_writable(FrameQueue *f);
// 获取可读
Frame *frame_queue_peek_readable(FrameQueue *f);
// 更新写指针
void frame_queue_push(FrameQueue *f);
/* 释放当前frame，并更新读索引rindex */
void frame_queue_next(FrameQueue *f);
int frame_queue_nb_remaining(FrameQueue *f);
int64_t frame_queue_last_pos(FrameQueue *f);


enum {
    AV_SYNC_UNKNOW_MASTER = -1,
    AV_SYNC_AUDIO_MASTER,
    AV_SYNC_VIDEO_MASTER
};

class Clock{
public:
    Clock();
    double get_clock();
    void set_clock_at(double pts, double time);
    void set_clock(double pts);
    void set_pause_val(int val){ pause = val;}
    int get_pause(){ return pause;}
private:
    double pts;
    double pts_drift;
    double last_updated;
    int serial;
    std::atomic<int> pause;
};

#endif // FFPLAY_DEF_H
