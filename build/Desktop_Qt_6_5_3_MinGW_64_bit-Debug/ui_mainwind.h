/********************************************************************************
** Form generated from reading UI file 'mainwind.ui'
**
** Created by: Qt User Interface Compiler version 6.5.3
**
** WARNING! All changes made in this file will be lost when recompiling UI file!
********************************************************************************/

#ifndef UI_MAINWIND_H
#define UI_MAINWIND_H

#include <QtCore/QVariant>
#include <QtWidgets/QApplication>
#include <QtWidgets/QDockWidget>
#include <QtWidgets/QMainWindow>
#include <QtWidgets/QMenuBar>
#include <QtWidgets/QStatusBar>
#include <QtWidgets/QWidget>
#include <ctrlbar.h>

QT_BEGIN_NAMESPACE

class Ui_MainWind
{
public:
    QWidget *centralwidget;
    QWidget *showCtrlBarBgWidget;
    CtrlBar *ctrlBarWidget;
    QWidget *showWidget;
    QMenuBar *menubar;
    QStatusBar *statusbar;
    QDockWidget *playListdockWidget;
    QWidget *playListContents;
    QDockWidget *titleDockWidget;
    QWidget *titleContents;

    void setupUi(QMainWindow *MainWind)
    {
        if (MainWind->objectName().isEmpty())
            MainWind->setObjectName("MainWind");
        MainWind->resize(1280, 800);
        centralwidget = new QWidget(MainWind);
        centralwidget->setObjectName("centralwidget");
        showCtrlBarBgWidget = new QWidget(centralwidget);
        showCtrlBarBgWidget->setObjectName("showCtrlBarBgWidget");
        showCtrlBarBgWidget->setGeometry(QRect(220, 50, 911, 551));
        ctrlBarWidget = new CtrlBar(showCtrlBarBgWidget);
        ctrlBarWidget->setObjectName("ctrlBarWidget");
        ctrlBarWidget->setGeometry(QRect(20, 479, 881, 61));
        showWidget = new QWidget(showCtrlBarBgWidget);
        showWidget->setObjectName("showWidget");
        showWidget->setGeometry(QRect(50, 90, 681, 391));
        MainWind->setCentralWidget(centralwidget);
        menubar = new QMenuBar(MainWind);
        menubar->setObjectName("menubar");
        menubar->setGeometry(QRect(0, 0, 1280, 18));
        MainWind->setMenuBar(menubar);
        statusbar = new QStatusBar(MainWind);
        statusbar->setObjectName("statusbar");
        MainWind->setStatusBar(statusbar);
        playListdockWidget = new QDockWidget(MainWind);
        playListdockWidget->setObjectName("playListdockWidget");
        playListContents = new QWidget();
        playListContents->setObjectName("playListContents");
        playListdockWidget->setWidget(playListContents);
        MainWind->addDockWidget(Qt::TopDockWidgetArea, playListdockWidget);
        titleDockWidget = new QDockWidget(MainWind);
        titleDockWidget->setObjectName("titleDockWidget");
        titleContents = new QWidget();
        titleContents->setObjectName("titleContents");
        titleDockWidget->setWidget(titleContents);
        MainWind->addDockWidget(Qt::TopDockWidgetArea, titleDockWidget);

        retranslateUi(MainWind);

        QMetaObject::connectSlotsByName(MainWind);
    } // setupUi

    void retranslateUi(QMainWindow *MainWind)
    {
        MainWind->setWindowTitle(QCoreApplication::translate("MainWind", "MainWind", nullptr));
    } // retranslateUi

};

namespace Ui {
    class MainWind: public Ui_MainWind {};
} // namespace Ui

QT_END_NAMESPACE

#endif // UI_MAINWIND_H
