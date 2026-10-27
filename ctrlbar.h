#ifndef CTRLBAR_H
#define CTRLBAR_H

#include <QWidget>

namespace Ui {
class CtrlBar;
}

class CtrlBar : public QWidget
{
    Q_OBJECT

public:
    explicit CtrlBar(QWidget *parent = nullptr);
    ~CtrlBar();

    void set_duration(long long micr);
    void set_now_position(long long micr);
signals:
    void SigPlayOrPause(); //开始暂停键信号
    void SigStop();

    void SigForward();//前进
    void SigBackward();//回退
private slots:
    void on_playOrPauseBtn_clicked();

    void on_stopBtn_clicked();

    void on_backwardBtn_clicked();

    void on_forwardBtn_clicked();

private:
    int pause = 0;
    QIcon* pause_icon;
    QIcon* play_icon;
    Ui::CtrlBar *ui;
};

#endif // CTRLBAR_H
