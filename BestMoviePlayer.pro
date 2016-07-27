#-------------------------------------------------
#
# Project created by QtCreator 2016-07-21T09:14:50
#
#-------------------------------------------------

QT       += core gui network webkitwidgets webview

greaterThan(QT_MAJOR_VERSION, 4): QT += widgets

TARGET = BestMoviePlayer
TEMPLATE = app

CONFIG += qt warn_on
CONFIG += release

DEFINES += SINGLE_INSTANCE
DEFINES += LOG_BESTMOVIEPLAYER


SOURCES += main.cpp\
        bestmovieplayer.cpp \
    myapplication.cpp \
    qtsingleapplication.cpp \
    qtlocalpeer.cpp \
    about.cpp \
    autohidewidget.cpp \
    base64.cpp \
    core.cpp \
    dcpdata.cpp \
    dcpplayer.cpp \
    desktopinfo.cpp \
    global.cpp \
    helper.cpp \
    images.cpp \
    kdm.cpp \
    mediasettings.cpp \
    mplayerwindow.cpp \
    myaction.cpp \
    myactiongroup.cpp \
    myplayerprocess.cpp \
    myslider.cpp \
    mywebmessage.cpp \
    mywidgetaction.cpp \
    paths.cpp \
    playlist.cpp \
    preference.cpp \
    rsa.cpp \
    socketconnect.cpp \
    timeslider.cpp \
    tinystr.cpp \
    tinyxml.cpp \
    tinyxmlerror.cpp \
    tinyxmlparser.cpp \
    webpage.cpp \
    aes.c \
    applink.c \
    version.cpp \
    qtlockedfile.cpp \
    qtlockedfile_win.cpp

HEADERS  += bestmovieplayer.h \
    myapplication.h \
    qtsingleapplication.h \
    qtlocalpeer.h \
    about.h \
    aes.h \
    autohidewidget.h \
    base64.h \
    config.h \
    constants.h \
    core.h \
    dcpdata.h \
    dcpplayer.h \
    desktopinfo.h \
    global.h \
    helper.h \
    images.h \
    kdm.h \
    mediasettings.h \
    mplayerwindow.h \
    myaction.h \
    myactiongroup.h \
    myplayerprocess.h \
    myslider.h \
    mywebmessage.h \
    mywidgetaction.h \
    paths.h \
    playlist.h \
    preference.h \
    rsa.h \
    socketconnect.h \
    timeslider.h \
    tinystr.h \
    tinyxml.h \
    webpage.h \
    version.h \
    qtlockedfile.h

FORMS += \
    about.ui

RESOURCES += \
    icons.qrc

LIBS += -Wl,-dy -LD:/msys2/mingw32/lib -lssl -lcrypto -lnfs -lws2_32

INCLUDEPATH += D:/msys2/mingw32/include

DISTFILES += \
    myRc.rc

RC_FILE += \
    myRc.rc
