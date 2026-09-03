/********************************************************************************
** Form generated from reading UI file 'displaywind.ui'
**
** Created by: Qt User Interface Compiler version 6.5.3
**
** WARNING! All changes made in this file will be lost when recompiling UI file!
********************************************************************************/

#ifndef UI_DISPLAYWIND_H
#define UI_DISPLAYWIND_H

#include <QtCore/QVariant>
#include <QtWidgets/QApplication>
#include <QtWidgets/QGridLayout>
#include <QtWidgets/QLabel>
#include <QtWidgets/QWidget>

QT_BEGIN_NAMESPACE

class Ui_DisplayWind
{
public:
    QGridLayout *gridLayout;
    QLabel *label;

    void setupUi(QWidget *DisplayWind)
    {
        if (DisplayWind->objectName().isEmpty())
            DisplayWind->setObjectName("DisplayWind");
        DisplayWind->resize(670, 431);
        gridLayout = new QGridLayout(DisplayWind);
        gridLayout->setSpacing(0);
        gridLayout->setObjectName("gridLayout");
        gridLayout->setContentsMargins(0, 0, 0, 0);
        label = new QLabel(DisplayWind);
        label->setObjectName("label");

        gridLayout->addWidget(label, 0, 0, 1, 1);


        retranslateUi(DisplayWind);

        QMetaObject::connectSlotsByName(DisplayWind);
    } // setupUi

    void retranslateUi(QWidget *DisplayWind)
    {
        DisplayWind->setWindowTitle(QCoreApplication::translate("DisplayWind", "Form", nullptr));
        label->setText(QCoreApplication::translate("DisplayWind", "\350\247\206\351\242\221\347\224\273\351\235\242", nullptr));
    } // retranslateUi

};

namespace Ui {
    class DisplayWind: public Ui_DisplayWind {};
} // namespace Ui

QT_END_NAMESPACE

#endif // UI_DISPLAYWIND_H
