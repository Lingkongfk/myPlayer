QT += widgets

CONFIG += c++17

# You can make your code fail to compile if it uses deprecated APIs.
# In order to do so, uncomment the following line.
#DEFINES += QT_DISABLE_DEPRECATED_BEFORE=0x060000    # disables all the APIs deprecated before Qt 6.0.0



SOURCES += \
    ctrlbar.cpp \
    displaywind.cpp \
    ffmsg_queue.cpp \
    main.cpp \
    mainwind.cpp \
    playlistwind.cpp \
    titlebar.cpp

HEADERS += \
    ctrlbar.h \
    displaywind.h \
    ffmsg.h \
    ffmsg_queue.h \
    mainwind.h \
    playlistwind.h \
    titlebar.h

FORMS += \
    ctrlbar.ui \
    displaywind.ui \
    mainwind.ui \
    playlistwind.ui \
    titlebar.ui

# Default rules for deployment.
qnx: target.path = /tmp/$${TARGET}/bin
else: unix:!android: target.path = /opt/$${TARGET}/bin
!isEmpty(target.path): INSTALLS += target

RESOURCES += \
    res.qrc


INCLUDEPATH += D:/ffmpeg-sdk/include

LIBS += -LD:/ffmpeg-sdk/lib \
        -lavformat \
        -lavcodec \
        -lavutil \
        -lavfilter \
        -lswresample \
        -lswscale \
        -lavdevice