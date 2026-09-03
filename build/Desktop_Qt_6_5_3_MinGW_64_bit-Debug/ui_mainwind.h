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
#include <QtWidgets/QGridLayout>
#include <QtWidgets/QMainWindow>
#include <QtWidgets/QMenuBar>
#include <QtWidgets/QStatusBar>
#include <QtWidgets/QWidget>
#include <ctrlbar.h>
#include <displaywind.h>
#include <playlistwind.h>
#include <titlebar.h>

QT_BEGIN_NAMESPACE

class Ui_MainWind
{
public:
    QWidget *centralwidget;
    QGridLayout *gridLayout;
    QWidget *showCtrlBarBgWidget;
    QGridLayout *gridLayout_2;
    DisplayWind *showWidget;
    CtrlBar *ctrlBarWidget;
    QMenuBar *menubar;
    QStatusBar *statusbar;
    QDockWidget *playListdockWidget;
    PlayListWind *playListContents;
    QDockWidget *titleDockWidget;
    TitleBar *titleContents;

    void setupUi(QMainWindow *MainWind)
    {
        if (MainWind->objectName().isEmpty())
            MainWind->setObjectName("MainWind");
        MainWind->resize(1280, 800);
        centralwidget = new QWidget(MainWind);
        centralwidget->setObjectName("centralwidget");
        gridLayout = new QGridLayout(centralwidget);
        gridLayout->setSpacing(0);
        gridLayout->setObjectName("gridLayout");
        gridLayout->setContentsMargins(0, 0, 0, 0);
        showCtrlBarBgWidget = new QWidget(centralwidget);
        showCtrlBarBgWidget->setObjectName("showCtrlBarBgWidget");
        gridLayout_2 = new QGridLayout(showCtrlBarBgWidget);
        gridLayout_2->setSpacing(0);
        gridLayout_2->setObjectName("gridLayout_2");
        gridLayout_2->setContentsMargins(0, 0, 0, 0);
        showWidget = new DisplayWind(showCtrlBarBgWidget);
        showWidget->setObjectName("showWidget");

        gridLayout_2->addWidget(showWidget, 0, 0, 1, 1);

        ctrlBarWidget = new CtrlBar(showCtrlBarBgWidget);
        ctrlBarWidget->setObjectName("ctrlBarWidget");
        ctrlBarWidget->setMinimumSize(QSize(300, 60));

        gridLayout_2->addWidget(ctrlBarWidget, 1, 0, 1, 1);


        gridLayout->addWidget(showCtrlBarBgWidget, 0, 0, 1, 1);

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
        playListdockWidget->setMinimumSize(QSize(100, 118));
        playListContents = new PlayListWind();
        playListContents->setObjectName("playListContents");
        playListContents->setMinimumSize(QSize(100, 100));
        playListdockWidget->setWidget(playListContents);
        MainWind->addDockWidget(Qt::RightDockWidgetArea, playListdockWidget);
        titleDockWidget = new QDockWidget(MainWind);
        titleDockWidget->setObjectName("titleDockWidget");
        titleDockWidget->setMinimumSize(QSize(200, 34));
        titleContents = new TitleBar();
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
