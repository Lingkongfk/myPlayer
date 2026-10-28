#include "ctrlbar.h"
#include "ui_ctrlbar.h"
#include <QDebug>

CtrlBar::CtrlBar(QWidget *parent)
    : QWidget(parent)
    , ui(new Ui::CtrlBar)
{
    ui->setupUi(this);

    pause_icon = new QIcon(":/images/pause.png");

    //设置播放图标
    play_icon = new QIcon(":/images/play.png");
    ui->playOrPauseBtn->setIcon(*play_icon);
    ui->playOrPauseBtn->setText("");
    //设置停止图标
    QIcon stop_icon = QIcon(":/images/stop.png");
    ui->stopBtn->setIcon(stop_icon);
    ui->stopBtn->setText("");
    //设置回退按钮
    QIcon backward_icon = QIcon(":/images/backward.png");
    ui->backwardBtn->setIcon(backward_icon);
    ui->backwardBtn->setText("");
    //设置前进图标
    QIcon forward_icon = QIcon(":/images/forward.png");
    ui->forwardBtn->setIcon(forward_icon);
    ui->forwardBtn->setText("");
    //设置播放列表
    QIcon playList_icon = QIcon(":/images/list.png");
    ui->playListBtn->setIcon(playList_icon);
    ui->playListBtn->setText("");
    //设置设置图标
    QIcon setting_icon = QIcon(":/images/setting.png");
    ui->settingBtn->setIcon(setting_icon);
    ui->settingBtn->setText("");

}

CtrlBar::~CtrlBar()
{
    delete ui;
}

void CtrlBar::set_duration(long long micr)
{
    QTime time = QTime::fromMSecsSinceStartOfDay(micr / 1000);
    ui->totalTimeEdit->setTime(time);
    ui->playSlider->setRange(0, micr / 1000000 + 1);//进度条范围也是秒为单位
}

void CtrlBar::set_now_position(long long micr)
{
    QTime time = QTime::fromMSecsSinceStartOfDay(micr);
    ui->playTimeEdit->setTime(time);
}

void CtrlBar::set_now_slider(long long second)
{
    if(!ui->playSlider->isSliderDown()){
        ui->playSlider->setValue(second);
    }
}

void CtrlBar::on_playOrPauseBtn_clicked()
{
    qDebug() << "on_playOrPauseBtn_clicked";
    if(pause){
        pause = 0;
        ui->playOrPauseBtn->setIcon(*play_icon);
    }else{
        pause = 1;
        ui->playOrPauseBtn->setIcon(*pause_icon);
    }
    emit SigPlayOrPause();
}


void CtrlBar::on_stopBtn_clicked()
{
    qDebug() << "on_stopBtn_clicked";
    emit SigStop();
}


void CtrlBar::on_backwardBtn_clicked()
{
    emit SigBackward();
}


void CtrlBar::on_forwardBtn_clicked()
{
    emit SigForward();
}


void CtrlBar::on_playSlider_sliderReleased()
{
    emit SigSeek(now_slider_time);
}


void CtrlBar::on_playSlider_valueChanged(int value)
{
    now_slider_time = value;
}

