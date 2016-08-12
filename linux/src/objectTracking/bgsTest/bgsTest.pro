#-------------------------------------------------
#
# Project created by QtCreator 2016-08-11T16:38:44
#
#-------------------------------------------------

QT       += core gui

greaterThan(QT_MAJOR_VERSION, 4): QT += widgets
CONFIG +=c++11

TARGET = bgsTest
TEMPLATE = app


SOURCES += main.cpp\
        bgstest.cpp \
    ../lib/bgsfactory_yzbx.cpp

HEADERS  += bgstest.h \
    ../lib/bgsfactory_yzbx.h

FORMS    += bgstest.ui

unix: CONFIG += link_pkgconfig
unix: PKGCONFIG += opencv

unix: PKGCONFIG += bgslibrary
