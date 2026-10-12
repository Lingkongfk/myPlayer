#ifndef DISPLAYWIND_H
#define DISPLAYWIND_H

#include <QWidget>
#include <QMutex>
#include "mymediaplayer.h"
#include "imagescaler.h"

namespace Ui {
class DisplayWind;
}

class DisplayWind : public QWidget
{
    Q_OBJECT

public:
    explicit DisplayWind(QWidget *parent = nullptr);
    ~DisplayWind();

    int Draw(const Frame* frame);
protected:
    void paintEvent(QPaintEvent *event)override;
    void resizeEvent(QResizeEvent* event);

private:
    Ui::DisplayWind *ui;

    int m_nLastFrameWidth;
    int m_nLastFrameHeight;
    bool is_display_size_change = false;

    int x_ = 0;
    int y_ = 0;
    int video_width = 0;
    int video_height = 0;
    int img_width = 0;
    int img_height = 0;
    QImage img;
    VideoFrame dst_video_frame_;
    QMutex m_mutex;
    ImageScaler* img_scaler = NULL;

};

#endif // DISPLAYWIND_H
