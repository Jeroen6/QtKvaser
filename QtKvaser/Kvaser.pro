#-------------------------------------------------
#
# Project created by QtCreator 2015-05-31T15:38:16
#
#-------------------------------------------------

QT       += core gui

greaterThan(QT_MAJOR_VERSION, 4): QT += widgets

TARGET = QtKvaser
TEMPLATE = app


SOURCES += main.cpp\
        mainwindow.cpp

HEADERS  += mainwindow.h \
    predef.h \
    obsolete.h \
    canstat.h \
    canlib.h \
    canevt.h

FORMS    += mainwindow.ui

LIBS += canlib32.dll

message("Please copy *.dll's from source to build directory")

DISTFILES += \
    canlib32.dll
