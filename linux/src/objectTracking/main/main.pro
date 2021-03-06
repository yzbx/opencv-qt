#-------------------------------------------------
#
# Project created by QtCreator 2016-08-09T20:53:03
#
#-------------------------------------------------

QT       += core gui

greaterThan(QT_MAJOR_VERSION, 4): QT += widgets
CONFIG +=c++11
QMAKE_CXXFLAGS += "-g -ftest-coverage -fprofile-arcs -msse -msse2 -msse3 -march=native"
QMAKE_LFLAGS +="-lgcov --coverage"

TARGET = main
TEMPLATE = app


SOURCES += main.cpp\
        mainwindow.cpp \
    ../lib/bgsfactory_yzbx.cpp \
    ../extern/qt-json/json.cpp \
    ../lib/trackingfactory_yzbx.cpp \
    ../lib/urbantracker_tracking.cpp

HEADERS  += mainwindow.h \
    ../lib/bgsfactory_yzbx.h \
    ../extern/qt-json/json.h \
    ../lib/trackingfactory_yzbx.h \
    ../lib/tracking_yzbx.h \
    ../lib/urbantracker_tracking.h

INCLUDEPATH +=/usr/local/include/UrbanTrackerLib/include \
    /usr/local/include/UrbanTrackerLib/include/external \
    /usr/local/include/UrbanTrackerLib/include/external/brisk \
    /usr/local/include/UrbanTrackerLib/include/external/brisk/include \
    /usr/local/include/UrbanTrackerLib/include/external/cvBlob/include \
    /usr/local/include/UrbanTrackerLib/include/external/sqlite/include \

FORMS    += mainwindow.ui

unix: CONFIG += link_pkgconfig
unix: PKGCONFIG += opencv bgslibrary

LIBS += \
    -lboost_system \
    -lboost_program_options \
    -lboost_filesystem \
