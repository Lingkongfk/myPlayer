/********************************************************************************
** Form generated from reading UI file 'ctrlbar.ui'
**
** Created by: Qt User Interface Compiler version 6.5.3
**
** WARNING! All changes made in this file will be lost when recompiling UI file!
********************************************************************************/

#ifndef UI_CTRLBAR_H
#define UI_CTRLBAR_H

#include <QtCore/QVariant>
#include <QtWidgets/QApplication>
#include <QtWidgets/QGridLayout>
#include <QtWidgets/QHBoxLayout>
#include <QtWidgets/QLabel>
#include <QtWidgets/QPushButton>
#include <QtWidgets/QSlider>
#include <QtWidgets/QSpacerItem>
#include <QtWidgets/QTimeEdit>
#include <QtWidgets/QVBoxLayout>
#include <QtWidgets/QWidget>

QT_BEGIN_NAMESPACE

class Ui_CtrlBar
{
public:
    QVBoxLayout *verticalLayout;
    QWidget *widget;
    QHBoxLayout *horizontalLayout_2;
    QSlider *playSlider;
    QWidget *widget_2;
    QHBoxLayout *horizontalLayout;
    QPushButton *volumeBtn;
    QSlider *volumeSlider;
    QGridLayout *gridLayout;
    QPushButton *playOrPauseBtn;
    QPushButton *backwardBtn;
    QPushButton *playListBtn;
    QPushButton *forwardBtn;
    QPushButton *stopBtn;
    QPushButton *settingBtn;
    QLabel *label;
    QPushButton *speedBtn;
    QTimeEdit *totalTimeEdit;
    QTimeEdit *playTimeEdit;
    QSpacerItem *horizontalSpacer;

    void setupUi(QWidget *CtrlBar)
    {
        if (CtrlBar->objectName().isEmpty())
            CtrlBar->setObjectName("CtrlBar");
        CtrlBar->resize(510, 80);
        CtrlBar->setMaximumSize(QSize(16777215, 80));
        verticalLayout = new QVBoxLayout(CtrlBar);
        verticalLayout->setSpacing(0);
        verticalLayout->setObjectName("verticalLayout");
        verticalLayout->setContentsMargins(8, 0, 0, 0);
        widget = new QWidget(CtrlBar);
        widget->setObjectName("widget");
        horizontalLayout_2 = new QHBoxLayout(widget);
        horizontalLayout_2->setSpacing(0);
        horizontalLayout_2->setObjectName("horizontalLayout_2");
        horizontalLayout_2->setContentsMargins(0, 0, 0, 0);
        playSlider = new QSlider(widget);
        playSlider->setObjectName("playSlider");
        playSlider->setOrientation(Qt::Horizontal);

        horizontalLayout_2->addWidget(playSlider);

        widget_2 = new QWidget(widget);
        widget_2->setObjectName("widget_2");
        widget_2->setMinimumSize(QSize(0, 0));
        widget_2->setMaximumSize(QSize(120, 40));
        horizontalLayout = new QHBoxLayout(widget_2);
        horizontalLayout->setSpacing(0);
        horizontalLayout->setObjectName("horizontalLayout");
        horizontalLayout->setContentsMargins(0, 0, 0, 0);
        volumeBtn = new QPushButton(widget_2);
        volumeBtn->setObjectName("volumeBtn");
        volumeBtn->setMinimumSize(QSize(20, 20));
        volumeBtn->setMaximumSize(QSize(20, 20));

        horizontalLayout->addWidget(volumeBtn);

        volumeSlider = new QSlider(widget_2);
        volumeSlider->setObjectName("volumeSlider");
        volumeSlider->setMinimumSize(QSize(60, 0));
        volumeSlider->setMaximumSize(QSize(80, 20));
        volumeSlider->setOrientation(Qt::Horizontal);

        horizontalLayout->addWidget(volumeSlider);


        horizontalLayout_2->addWidget(widget_2);


        verticalLayout->addWidget(widget);

        gridLayout = new QGridLayout();
        gridLayout->setSpacing(0);
        gridLayout->setObjectName("gridLayout");
        playOrPauseBtn = new QPushButton(CtrlBar);
        playOrPauseBtn->setObjectName("playOrPauseBtn");
        playOrPauseBtn->setMinimumSize(QSize(30, 30));
        playOrPauseBtn->setMaximumSize(QSize(30, 30));

        gridLayout->addWidget(playOrPauseBtn, 0, 0, 1, 1);

        backwardBtn = new QPushButton(CtrlBar);
        backwardBtn->setObjectName("backwardBtn");
        backwardBtn->setMinimumSize(QSize(30, 30));
        backwardBtn->setMaximumSize(QSize(30, 30));

        gridLayout->addWidget(backwardBtn, 0, 3, 1, 1);

        playListBtn = new QPushButton(CtrlBar);
        playListBtn->setObjectName("playListBtn");
        playListBtn->setMinimumSize(QSize(30, 30));
        playListBtn->setMaximumSize(QSize(30, 30));

        gridLayout->addWidget(playListBtn, 0, 9, 1, 1);

        forwardBtn = new QPushButton(CtrlBar);
        forwardBtn->setObjectName("forwardBtn");
        forwardBtn->setMinimumSize(QSize(30, 30));
        forwardBtn->setMaximumSize(QSize(30, 30));

        gridLayout->addWidget(forwardBtn, 0, 2, 1, 1);

        stopBtn = new QPushButton(CtrlBar);
        stopBtn->setObjectName("stopBtn");
        stopBtn->setMinimumSize(QSize(30, 30));
        stopBtn->setMaximumSize(QSize(30, 30));

        gridLayout->addWidget(stopBtn, 0, 1, 1, 1);

        settingBtn = new QPushButton(CtrlBar);
        settingBtn->setObjectName("settingBtn");
        settingBtn->setMinimumSize(QSize(30, 30));
        settingBtn->setMaximumSize(QSize(30, 30));

        gridLayout->addWidget(settingBtn, 0, 10, 1, 1);

        label = new QLabel(CtrlBar);
        label->setObjectName("label");
        label->setMaximumSize(QSize(20, 16777215));

        gridLayout->addWidget(label, 0, 6, 1, 1);

        speedBtn = new QPushButton(CtrlBar);
        speedBtn->setObjectName("speedBtn");
        speedBtn->setMinimumSize(QSize(30, 30));
        speedBtn->setMaximumSize(QSize(30, 30));

        gridLayout->addWidget(speedBtn, 0, 4, 1, 1);

        totalTimeEdit = new QTimeEdit(CtrlBar);
        totalTimeEdit->setObjectName("totalTimeEdit");
        totalTimeEdit->setButtonSymbols(QAbstractSpinBox::NoButtons);

        gridLayout->addWidget(totalTimeEdit, 0, 7, 1, 1);

        playTimeEdit = new QTimeEdit(CtrlBar);
        playTimeEdit->setObjectName("playTimeEdit");
        playTimeEdit->setMaximumSize(QSize(90, 16777215));
        playTimeEdit->setButtonSymbols(QAbstractSpinBox::NoButtons);

        gridLayout->addWidget(playTimeEdit, 0, 5, 1, 1);

        horizontalSpacer = new QSpacerItem(40, 20, QSizePolicy::Expanding, QSizePolicy::Minimum);

        gridLayout->addItem(horizontalSpacer, 0, 8, 1, 1);


        verticalLayout->addLayout(gridLayout);


        retranslateUi(CtrlBar);

        QMetaObject::connectSlotsByName(CtrlBar);
    } // setupUi

    void retranslateUi(QWidget *CtrlBar)
    {
        CtrlBar->setWindowTitle(QCoreApplication::translate("CtrlBar", "Form", nullptr));
        volumeBtn->setText(QCoreApplication::translate("CtrlBar", "PushButton", nullptr));
        playOrPauseBtn->setText(QCoreApplication::translate("CtrlBar", "PushButton", nullptr));
        backwardBtn->setText(QCoreApplication::translate("CtrlBar", "PushButton", nullptr));
        playListBtn->setText(QCoreApplication::translate("CtrlBar", "PushButton", nullptr));
        forwardBtn->setText(QCoreApplication::translate("CtrlBar", "PushButton", nullptr));
        stopBtn->setText(QCoreApplication::translate("CtrlBar", "PushButton", nullptr));
        settingBtn->setText(QCoreApplication::translate("CtrlBar", "PushButton", nullptr));
        label->setText(QCoreApplication::translate("CtrlBar", "/", nullptr));
        speedBtn->setText(QCoreApplication::translate("CtrlBar", "PushButton", nullptr));
        totalTimeEdit->setDisplayFormat(QCoreApplication::translate("CtrlBar", "HH:mm:ss", nullptr));
        playTimeEdit->setDisplayFormat(QCoreApplication::translate("CtrlBar", "HH:mm:ss", nullptr));
    } // retranslateUi

};

namespace Ui {
    class CtrlBar: public Ui_CtrlBar {};
} // namespace Ui

QT_END_NAMESPACE

#endif // UI_CTRLBAR_H
