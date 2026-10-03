QT += widgets

CONFIG += c++17

# You can make your code fail to compile if it uses deprecated APIs.
# In order to do so, uncomment the following line.
#DEFINES += QT_DISABLE_DEPRECATED_BEFORE=0x060000    # disables all the APIs deprecated before Qt 6.0.0



SOURCES += \
    ctrlbar.cpp \
    displaywind.cpp \
    ffmsg_queue.cpp \
    ffplayer.cpp \
    main.cpp \
    mainwind.cpp \
    mymediaplayer.cpp \
    playlistwind.cpp \
    titlebar.cpp

HEADERS += \
    ctrlbar.h \
    displaywind.h \
    ffmsg.h \
    ffmsg_queue.h \
    ffplayer.h \
    mainwind.h \
    mymediaplayer.h \
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
INCLUDEPATH += D:/SDL2-2.30.12/include

LIBS += -LD:/ffmpeg-sdk/lib \
        -LD:/SDL2-2.30.12/lib/x64 \
        -lavformat \
        -lavcodec \
        -lavutil \
        -lavfilter \
        -lswresample \
        -lswscale \
        -lavdevice