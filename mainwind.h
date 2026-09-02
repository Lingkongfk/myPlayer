#ifndef MAINWIND_H
#define MAINWIND_H

#include <QMainWindow>

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

private:
    Ui::MainWind *ui;
};
#endif // MAINWIND_H
