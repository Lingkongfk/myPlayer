#include "ctrlbar.h"
#include "ui_ctrlbar.h"
#include <QDebug>

CtrlBar::CtrlBar(QWidget *parent)
    : QWidget(parent)
    , ui(new Ui::CtrlBar)
{
    ui->setupUi(this);

    //设置播放图标
    QIcon play_icon = QIcon(":/images/play.png");
    ui->playOrPauseBtn->setIcon(play_icon);
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

void CtrlBar::on_playOrPauseBtn_clicked()
{
    qDebug() << "on_playOrPauseBtn_clicked";
    emit SigPlayOrPause();
}


void CtrlBar::on_stopBtn_clicked()
{
    qDebug() << "on_stopBtn_clicked";
    emit SigStop();
}

