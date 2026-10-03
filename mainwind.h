#ifndef MAINWIND_H
#define MAINWIND_H

#include <QMainWindow>
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
private:
    Ui::MainWind *ui;
    MyMediaPlayer* mp_ = NULL;
};
#endif // MAINWIND_H
