/********************************************************************************
** Form generated from reading UI file 'playlistwind.ui'
**
** Created by: Qt User Interface Compiler version 6.5.3
**
** WARNING! All changes made in this file will be lost when recompiling UI file!
********************************************************************************/

#ifndef UI_PLAYLISTWIND_H
#define UI_PLAYLISTWIND_H

#include <QtCore/QVariant>
#include <QtWidgets/QApplication>
#include <QtWidgets/QListWidget>
#include <QtWidgets/QVBoxLayout>
#include <QtWidgets/QWidget>

QT_BEGIN_NAMESPACE

class Ui_PlayListWind
{
public:
    QVBoxLayout *verticalLayout;
    QListWidget *list;

    void setupUi(QWidget *PlayListWind)
    {
        if (PlayListWind->objectName().isEmpty())
            PlayListWind->setObjectName("PlayListWind");
        PlayListWind->resize(200, 200);
        PlayListWind->setMaximumSize(QSize(200, 16777215));
        verticalLayout = new QVBoxLayout(PlayListWind);
        verticalLayout->setObjectName("verticalLayout");
        list = new QListWidget(PlayListWind);
        new QListWidgetItem(list);
        new QListWidgetItem(list);
        list->setObjectName("list");

        verticalLayout->addWidget(list);


        retranslateUi(PlayListWind);

        QMetaObject::connectSlotsByName(PlayListWind);
    } // setupUi

    void retranslateUi(QWidget *PlayListWind)
    {
        PlayListWind->setWindowTitle(QCoreApplication::translate("PlayListWind", "Form", nullptr));

        const bool __sortingEnabled = list->isSortingEnabled();
        list->setSortingEnabled(false);
        QListWidgetItem *___qlistwidgetitem = list->item(0);
        ___qlistwidgetitem->setText(QCoreApplication::translate("PlayListWind", "\345\221\250\346\235\260\344\274\246-\345\217\214\346\210\252\346\243\215", nullptr));
        QListWidgetItem *___qlistwidgetitem1 = list->item(1);
        ___qlistwidgetitem1->setText(QCoreApplication::translate("PlayListWind", "\346\236\227\344\277\212\346\235\260-\347\276\216\344\272\272\351\261\274", nullptr));
        list->setSortingEnabled(__sortingEnabled);

    } // retranslateUi

};

namespace Ui {
    class PlayListWind: public Ui_PlayListWind {};
} // namespace Ui

QT_END_NAMESPACE

#endif // UI_PLAYLISTWIND_H
