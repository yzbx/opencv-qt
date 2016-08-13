#-------------------------------------------------
#
# Project created by QtCreator 2016-08-11T21:46:14
#
#-------------------------------------------------

QT       += core gui

greaterThan(QT_MAJOR_VERSION, 4): QT += widgets
CONFIG +=c++11
QMAKE_CXXFLAGS += "-g -ftest-coverage -fprofile-arcs -msse -msse2 -msse3 -march=native"
QMAKE_LFLAGS +="-lgcov --coverage"
TARGET = trackingTest
TEMPLATE = app


SOURCES += main.cpp\
        trackingtest.cpp \
    ../extern/UrbanTracker/external/brisk/src/agast5_8.cc \
    ../extern/UrbanTracker/external/brisk/src/agast5_8_nms.cc \
    ../extern/UrbanTracker/external/brisk/src/agast7_12d.cc \
    ../extern/UrbanTracker/external/brisk/src/agast7_12d_nms.cc \
    ../extern/UrbanTracker/external/brisk/src/agast7_12s.cc \
    ../extern/UrbanTracker/external/brisk/src/agast7_12s_nms.cc \
    ../extern/UrbanTracker/external/brisk/src/AstDetector.cc \
    ../extern/UrbanTracker/external/brisk/src/brisk.cpp \
    ../extern/UrbanTracker/external/brisk/src/nonMaximumSuppression.cc \
    ../extern/UrbanTracker/external/brisk/src/oast9_16.cc \
    ../extern/UrbanTracker/external/brisk/src/oast9_16_nms.cc \
    ../extern/UrbanTracker/external/cvBlob/src/cvaux.cpp \
    ../extern/UrbanTracker/external/cvBlob/src/cvblob.cpp \
    ../extern/UrbanTracker/external/cvBlob/src/cvcolor.cpp \
    ../extern/UrbanTracker/external/cvBlob/src/cvcontour.cpp \
    ../extern/UrbanTracker/external/cvBlob/src/cvlabel.cpp \
    ../extern/UrbanTracker/src/ApplicationContext.cpp \
    ../extern/UrbanTracker/src/Blob.cpp \
    ../extern/UrbanTracker/src/BlobDetector.cpp \
    ../extern/UrbanTracker/src/connectedcomponents.cpp \
    ../extern/UrbanTracker/src/DrawableTimer.cpp \
    ../extern/UrbanTracker/src/FeatureDetectorExtractorMatcher.cpp \
    ../extern/UrbanTracker/src/FeaturePoint.cpp \
    ../extern/UrbanTracker/src/FrameAssociation.cpp \
    ../extern/UrbanTracker/src/InputFrameListModule.cpp \
    ../extern/UrbanTracker/src/InputVideoFileModule.cpp \
    ../extern/UrbanTracker/src/IObject.cpp \
    ../extern/UrbanTracker/src/Logger.cpp \
    ../extern/UrbanTracker/src/munkres.cpp \
    ../extern/UrbanTracker/src/Object.cpp \
    ../extern/UrbanTracker/src/ObjectGroup.cpp \
    ../extern/UrbanTracker/src/ObjectModel.cpp \
    ../extern/UrbanTracker/src/ObjectModelGroup.cpp \
    ../extern/UrbanTracker/src/ObjectTypeManager.cpp \
    ../extern/UrbanTracker/src/OpenCVHelpers.cpp \
    ../extern/UrbanTracker/src/PointsBlob.cpp \
    ../extern/UrbanTracker/src/Rect.cpp \
    ../extern/UrbanTracker/src/SceneObject.cpp \
    ../extern/UrbanTracker/src/SQLiteManager.cpp \
    ../extern/UrbanTracker/src/StringHelpers.cpp \
    ../extern/UrbanTracker/src/Timer.cpp \
    ../extern/UrbanTracker/src/Track.cpp \
    ../extern/UrbanTracker/src/Tracker.cpp \
    ../extern/UrbanTracker/src/TrackerPersistance.cpp \
    ../extern/UrbanTracker/external/sqlite/src/sqlite3.c \
    ../extern/UrbanTracker/src/BGSFactory.cpp \
    ../extern/UrbanTracker/src/PBAS.cpp \
    ../extern/UrbanTracker/src/PlaybackBGS.cpp

HEADERS  += trackingtest.h \
    ../extern/UrbanTracker/external/brisk/include/agast/agast5_8.h \
    ../extern/UrbanTracker/external/brisk/include/agast/agast7_12d.h \
    ../extern/UrbanTracker/external/brisk/include/agast/agast7_12s.h \
    ../extern/UrbanTracker/external/brisk/include/agast/AstDetector.h \
    ../extern/UrbanTracker/external/brisk/include/agast/cvWrapper.h \
    ../extern/UrbanTracker/external/brisk/include/agast/oast9_16.h \
    ../extern/UrbanTracker/external/brisk/include/brisk/brisk.h \
    ../extern/UrbanTracker/external/brisk/include/brisk/hammingsse.hpp \
    ../extern/UrbanTracker/external/cvBlob/include/cvblob.h \
    ../extern/UrbanTracker/external/sqlite/include/sqlite3.h \
    ../extern/UrbanTracker/include/ApplicationContext.h \
    ../extern/UrbanTracker/include/Blob.h \
    ../extern/UrbanTracker/include/BlobDetector.h \
    ../extern/UrbanTracker/include/BlobRect.h \
    ../extern/UrbanTracker/include/BlobTrackerAlgorithmParams.h \
    ../extern/UrbanTracker/include/connectedcomponents.h \
    ../extern/UrbanTracker/include/DrawableTimer.h \
    ../extern/UrbanTracker/include/DrawingFlags.h \
    ../extern/UrbanTracker/include/FeatureDetectorExtractorMatcher.h \
    ../extern/UrbanTracker/include/FeaturePoint.h \
    ../extern/UrbanTracker/include/FrameAssociation.h \
    ../extern/UrbanTracker/include/IFeatureDetectorExtractorMatcher.h \
    ../extern/UrbanTracker/include/InputFrameListModule.h \
    ../extern/UrbanTracker/include/InputFrameProviderIface.h \
    ../extern/UrbanTracker/include/InputVideoFileModule.h \
    ../extern/UrbanTracker/include/IObject.h \
    ../extern/UrbanTracker/include/IObjectModel.h \
    ../extern/UrbanTracker/include/Logger.h \
    ../extern/UrbanTracker/include/matrix.h \
    ../extern/UrbanTracker/include/MatrixIO.h \
    ../extern/UrbanTracker/include/munkres.h \
    ../extern/UrbanTracker/include/Object.h \
    ../extern/UrbanTracker/include/ObjectGroup.h \
    ../extern/UrbanTracker/include/ObjectModel.h \
    ../extern/UrbanTracker/include/ObjectModelGroup.h \
    ../extern/UrbanTracker/include/ObjectState.h \
    ../extern/UrbanTracker/include/ObjectTypeManager.h \
    ../extern/UrbanTracker/include/OpenCVHelpers.h \
    ../extern/UrbanTracker/include/PointsBlob.h \
    ../extern/UrbanTracker/include/Rect.h \
    ../extern/UrbanTracker/include/SceneObject.h \
    ../extern/UrbanTracker/include/SQLiteManager.h \
    ../extern/UrbanTracker/include/StringHelpers.h \
    ../extern/UrbanTracker/include/Timer.h \
    ../extern/UrbanTracker/include/Track.h \
    ../extern/UrbanTracker/include/Tracker.h \
    ../extern/UrbanTracker/include/TrackerPersistance.h \
    ../extern/UrbanTracker/include/BGSFactory.h \
    ../extern/UrbanTracker/include/PBAS.h \
    ../extern/UrbanTracker/include/PlaybackBGS.h \
    ../extern/UrbanTracker/include/IBGSStatic.h

INCLUDEPATH +=../extern/UrbanTracker/include \
    ../extern/UrbanTracker/external/brisk/include \
    ../extern/UrbanTracker/external/brisk/include/agast \
    ../extern/UrbanTracker/external/brisk/include/brisk \
    ../extern/UrbanTracker/external/cvBlob/include \
    ../extern/UrbanTracker/external/sqlite/include

LIBS += -lboost_system \
    -lboost_program_options \
    -lboost_filesystem

FORMS    += trackingtest.ui

unix: CONFIG += link_pkgconfig
unix: PKGCONFIG += opencv bgslibrary
