QT       += core gui network multimedia

greaterThan(QT_MAJOR_VERSION, 4): QT += widgets openglwidgets

CONFIG += c++17

TARGET = JCMPlayer

TEMPLATE = app

macx{

}

win32{

   INCLUDEPATH += $$PWD/3rd/win32/FFmpeglib/include

   LIBS += -L$$PWD/3rd/win32/FFmpeglib/lib \
            -lavcodec \
            -lavdevice \
            -lavfilter \
           -lavformat \
           -lavutil \
           -lpostproc \
           -lswresample \
           -lswscale

}


SOURCES += \
    jcavcodechandler.cpp \
    main.cpp \
    jcmainwindow.cpp \
    videoopengl.cpp

HEADERS += \
    avsync.h \
    jcavcodechandler.h \
    jcdatedefine.h \
    jcmainwindow.h \
    videoopengl.h

FORMS += \
    jcmainwindow.ui


