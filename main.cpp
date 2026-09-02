#include "mainwind.h"

#include <QApplication>

int main(int argc, char *argv[])
{
    QApplication a(argc, argv);
    MainWind w;
    w.show();
    return QApplication::exec();
}
