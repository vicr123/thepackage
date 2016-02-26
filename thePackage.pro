#-------------------------------------------------
#
# Project created by QtCreator 2016-02-25T20:24:19
#
#-------------------------------------------------

QT       += core gui
CONFIG   += C++11

greaterThan(QT_MAJOR_VERSION, 4): QT += widgets

TARGET = thepackage
TEMPLATE = app


SOURCES += main.cpp\
        mainwindow.cpp \
    package.cpp \
    worker.cpp \
    updatewindow.cpp \
    updateworker.cpp

HEADERS  += mainwindow.h \
    package.h \
    worker.h \
    updatewindow.h \
    updateworker.h

FORMS    += mainwindow.ui \
    updatewindow.ui
