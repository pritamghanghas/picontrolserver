#-------------------------------------------------
#
# Project created by QtCreator 2015-06-20T02:54:26
#
#-------------------------------------------------

QT       += core

QT       -= gui

TARGET = picontrolserver
CONFIG   += console network
CONFIG   -= app_bundle
CONFIG += C++11 TUFAO1

TEMPLATE = app


SOURCES += main.cpp \
    mainhandler.cpp \
    pidiscoverybeacon.cpp

HEADERS += \
    mainhandler.h \
    constants.h \
    pidiscoverybeacon.h

OTHER_FILES += \
    usage.json

INSTALLS += $OTHER_FILES
