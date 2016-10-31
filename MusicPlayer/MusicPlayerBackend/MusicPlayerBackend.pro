TEMPLATE = app
CONFIG += console c++11
CONFIG -= app_bundle
CONFIG -= qt

INCLUDEPATH += \
    /usr/include/glib-2.0/ \
    /usr/include/gstreamer-1.0 \
    /usr/lib/x86_64-linux-gnu/glib-2.0/include/ \
    /usr/lib/x86_64-linux-gnu/gstreamer-1.0/include \

SOURCES += main.c \
    mediafilesbrowser.c \
    usblistener.c \
    playercore.c \
    gstplayer.c \
    sldplayer.c \
    multimediacache.c \
    multimediacacheusb.c \
    multimediacachefilesys.c

HEADERS += \
    mediafilesbrowser.h \
    usblistener.h \
    logger.h \
    common.h \
    playercore.h \
    gstplayer.h \
    sdlplayer.h \
    multimediacache.h \
    multimediacacheusb.h \
    multimediacachefilesys.h

LIBS += \
    -lSDL \
    -lSDL_mixer \
    -ludev \
    -lpthread\
    -lglib-2.0\
    -gstreamer-1.0 \
    `pkg-config --libs gstreamer-1.0` \
