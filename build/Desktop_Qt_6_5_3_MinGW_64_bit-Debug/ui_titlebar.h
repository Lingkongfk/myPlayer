/********************************************************************************
** Form generated from reading UI file 'titlebar.ui'
**
** Created by: Qt User Interface Compiler version 6.5.3
**
** WARNING! All changes made in this file will be lost when recompiling UI file!
********************************************************************************/

#ifndef UI_TITLEBAR_H
#define UI_TITLEBAR_H

#include <QtCore/QVariant>
#include <QtWidgets/QApplication>
#include <QtWidgets/QHBoxLayout>
#include <QtWidgets/QLabel>
#include <QtWidgets/QPushButton>
#include <QtWidgets/QWidget>

QT_BEGIN_NAMESPACE

class Ui_TitleBar
{
public:
    QHBoxLayout *horizontalLayout;
    QPushButton *MenuBtn;
    QLabel *movieNameLabel;
    QPushButton *minBtn;
    QPushButton *fullScreenBtn;
    QPushButton *closeBtn;

    void setupUi(QWidget *TitleBar)
    {
        if (TitleBar->objectName().isEmpty())
            TitleBar->setObjectName("TitleBar");
        TitleBar->resize(674, 42);
        TitleBar->setMaximumSize(QSize(16777215, 50));
        horizontalLayout = new QHBoxLayout(TitleBar);
        horizontalLayout->setObjectName("horizontalLayout");
        MenuBtn = new QPushButton(TitleBar);
        MenuBtn->setObjectName("MenuBtn");
        MenuBtn->setMinimumSize(QSize(80, 30));
        MenuBtn->setMaximumSize(QSize(80, 30));

        horizontalLayout->addWidget(MenuBtn);

        movieNameLabel = new QLabel(TitleBar);
        movieNameLabel->setObjectName("movieNameLabel");

        horizontalLayout->addWidget(movieNameLabel);

        minBtn = new QPushButton(TitleBar);
        minBtn->setObjectName("minBtn");
        minBtn->setMinimumSize(QSize(30, 30));
        minBtn->setMaximumSize(QSize(30, 30));

        horizontalLayout->addWidget(minBtn);

        fullScreenBtn = new QPushButton(TitleBar);
        fullScreenBtn->setObjectName("fullScreenBtn");
        fullScreenBtn->setMinimumSize(QSize(30, 30));
        fullScreenBtn->setMaximumSize(QSize(30, 30));

        horizontalLayout->addWidget(fullScreenBtn);

        closeBtn = new QPushButton(TitleBar);
        closeBtn->setObjectName("closeBtn");
        closeBtn->setMinimumSize(QSize(30, 30));
        closeBtn->setMaximumSize(QSize(30, 30));

        horizontalLayout->addWidget(closeBtn);


        retranslateUi(TitleBar);

        QMetaObject::connectSlotsByName(TitleBar);
    } // setupUi

    void retranslateUi(QWidget *TitleBar)
    {
        TitleBar->setWindowTitle(QCoreApplication::translate("TitleBar", "Form", nullptr));
        MenuBtn->setText(QCoreApplication::translate("TitleBar", "PushButton", nullptr));
        movieNameLabel->setText(QCoreApplication::translate("TitleBar", "TextLabel", nullptr));
        minBtn->setText(QCoreApplication::translate("TitleBar", "PushButton", nullptr));
        fullScreenBtn->setText(QCoreApplication::translate("TitleBar", "PushButton", nullptr));
        closeBtn->setText(QCoreApplication::translate("TitleBar", "PushButton", nullptr));
    } // retranslateUi

};

namespace Ui {
    class TitleBar: public Ui_TitleBar {};
} // namespace Ui

QT_END_NAMESPACE

#endif // UI_TITLEBAR_H
