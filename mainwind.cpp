#include "mainwind.h"
#include "ui_mainwind.h"

MainWind::MainWind(QWidget *parent)
    : QMainWindow(parent)
    , ui(new Ui::MainWind)
{
    ui->setupUi(this);
    QObject::connect(ui->ctrlBarWidget, &CtrlBar::SigPlayOrPause, this, &MainWind::OnPlayOrPause);
    QObject::connect(ui->ctrlBarWidget, &CtrlBar::SigStop, this, &MainWind::OnStop);
}

MainWind::~MainWind()
{
    delete ui;
}

//该函数会作为参数传递给mymediaplayer创建一个线程进行执行，负责获取mymediaplayer发送过来的消息处理
int MainWind::message_loop(void *arg)
{
    MyMediaPlayer* mp = (MyMediaPlayer*)arg;
    qDebug() << "message_loop info";
    while(true){
        AVMessage msg;
        //阻塞调用
        int ret = mp->get_msg(&msg, 1);//mymediaplayer封装的，实际上就是调用queue_msg_get
        if(ret < 0)
            break;
        switch(msg.what){
        case FFP_MSG_FLUSH:
            qDebug() << __FUNCTION__ << " FFP_MSG_FLUSH";
            break;
        case FFP_MSG_PREPARED://当前视频已经准备好了，那就播放
            std::cout << __FUNCTION__ << " FFP_MSG_PREPARED" << std::endl;
            mp->start();
            break;
        default:
            qDebug()  << __FUNCTION__ << " default " << msg.what ;
            break;
        }

        //销毁msg
        msg_free_res(&msg);
        //使用sleep模拟线程运行
        std::this_thread::sleep_for(std::chrono::seconds(1));
    }
    qDebug() << "message_loop leave";
}

void MainWind::OnPlayOrPause()
{
    qDebug() << "OnPlayOrPause call";
    int ret = 0;

    //如果没有创建播放器，说明是一个新播放的视频
    if(!mp_){
        mp_ = new MyMediaPlayer();
        //创建播放器，并且把循环函数传递出去
        ret = mp_->create(std::bind(&MainWind::message_loop, this, std::placeholders::_1));
        if(ret < 0){
            qDebug() << "media player create failed";
            delete mp_;
            mp_ = NULL;
            return ;
        }

        mp_->set_data_source("2_audio.mp4");//暂时先用着

        //创建播放器成功后，启动准备
        ret = mp_->prepare_async();
        if(ret < 0){
            qDebug() << "media player create failed and prepare failed";
            delete mp_;
            mp_ = NULL;
            return ;
        }
    }else{
        //播放器已经有了，这里就是暂停或者恢复播放了
    }
}

void MainWind::OnStop()
{
    qDebug() << "OnStop call";
    if(mp_){
        mp_->stop();
        mp_->destory();
        delete mp_;
        mp_ = NULL;
    }
}
