#ifndef MAINWIND_H
#define MAINWIND_H

#include <QMainWindow>
#include <QTimer>
#include <atomic>
#include "mymediaplayer.h"
QT_BEGIN_NAMESPACE
namespace Ui {
class MainWind;
}
QT_END_NAMESPACE

class MainWind : public QMainWindow
{
    Q_OBJECT

public:
    explicit MainWind(QWidget *parent = nullptr);
    ~MainWind() override;

    int message_loop(void* arg);//不断获取消息队列消息的循环
    void OnPlayOrPause();//处理开始和暂停键按下时的操作
    void OnStop();//处理暂停按钮
    void OnForward();
    void OnBackward();
    void OnSeek(long long seek_to_time);

    void get_duration(long long micr);
    int outputVideo(const Frame* frame);//视频帧展示
private:
    Ui::MainWind *ui;
    MyMediaPlayer* mp_ = NULL;
    QTimer* timer = nullptr;
    std::atomic<long long> now_time = 0;//单位毫秒
};
#endif // MAINWIND_H
