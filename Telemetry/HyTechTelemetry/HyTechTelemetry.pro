#-------------------------------------------------
#
# Project created by QtCreator 2018-03-01T19:42:14
#
#-------------------------------------------------

QT       += core gui

greaterThan(QT_MAJOR_VERSION, 4): QT += widgets

TARGET = HyTechTelemetry
TEMPLATE = app

# The following define makes your compiler emit warnings if you use
# any feature of Qt which has been marked as deprecated (the exact warnings
# depend on your compiler). Please consult the documentation of the
# deprecated API in order to know how to port your code away from it.
DEFINES += QT_DEPRECATED_WARNINGS

# You can also make your code fail to compile if you use deprecated APIs.
# In order to do so, uncomment the following line.
# You can also select to disable deprecated APIs only up to a certain version of Qt.
#DEFINES += QT_DISABLE_DEPRECATED_BEFORE=0x060000    # disables all the APIs deprecated before Qt 6.0.0


SOURCES += \
        main.cpp \
        mc_fault_info.cpp \
        mainwindow.cpp \
    xbee_rcv_thread.cpp \
    ../../Libraries/HyTech17_Library/*.cpp \
    ../../Libraries/XBTools/XBTools.cpp \

HEADERS += \
        mc_fault_info.h \
        mainwindow.h \
    xbee_rcv_thread.h \

FORMS += \
        mainwindow.ui \
    mc_fault_info.ui
