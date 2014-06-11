#-------------------------------------------------
#
# Project created by QtCreator 2014-06-08T16:04:16
#
#-------------------------------------------------

QT       += core gui
greaterThan(QT_MAJOR_VERSION, 4): QT += printsupport widgets

TARGET = netPerfGUI
TEMPLATE = app


SOURCES += main.cpp\
        mainwindow.cpp \
    qcustomplot.cpp

HEADERS  += mainwindow.h \
    qcustomplot.h

FORMS    += mainwindow.ui

RESOURCES += \
    res.qrc
