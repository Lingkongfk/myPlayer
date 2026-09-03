#ifndef DISPLAYWIND_H
#define DISPLAYWIND_H

#include <QWidget>

namespace Ui {
class DisplayWind;
}

class DisplayWind : public QWidget
{
    Q_OBJECT

public:
    explicit DisplayWind(QWidget *parent = nullptr);
    ~DisplayWind();

private:
    Ui::DisplayWind *ui;
};

#endif // DISPLAYWIND_H
