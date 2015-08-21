#-------------------------------------------------
#
# Project created by QtCreator 2015-07-02T16:42:12
#
#-------------------------------------------------

QT       +=sql core gui

greaterThan(QT_MAJOR_VERSION, 4): QT += widgets

TARGET = opencv
TEMPLATE = app


SOURCES += main.cpp\
        mainwindow.cpp

HEADERS  += mainwindow.h

FORMS    += mainwindow.ui \
    videotest.ui

INCLUDEPATH+=D:\yzbx_allthing\build\install\include\
            D:\yzbx_allthing\build\install\include\opencv2\
            D:\yzbx_allthing\build\install\include\opencv\


LIBS+=D:\yzbx_allthing\build\install\x64\mingw\lib\libopencv_calib3d2411.dll.a\
D:\yzbx_allthing\build\install\x64\mingw\lib\libopencv_contrib2411.dll.a\
D:\yzbx_allthing\build\install\x64\mingw\lib\libopencv_core2411.dll.a\
D:\yzbx_allthing\build\install\x64\mingw\lib\libopencv_features2d2411.dll.a\
D:\yzbx_allthing\build\install\x64\mingw\lib\libopencv_flann2411.dll.a\
D:\yzbx_allthing\build\install\x64\mingw\lib\libopencv_gpu2411.dll.a\
D:\yzbx_allthing\build\install\x64\mingw\lib\libopencv_highgui2411.dll.a\
D:\yzbx_allthing\build\install\x64\mingw\lib\libopencv_imgproc2411.dll.a\
D:\yzbx_allthing\build\install\x64\mingw\lib\libopencv_legacy2411.dll.a\
D:\yzbx_allthing\build\install\x64\mingw\lib\libopencv_ml2411.dll.a\
D:\yzbx_allthing\build\install\x64\mingw\lib\libopencv_nonfree2411.dll.a\
D:\yzbx_allthing\build\install\x64\mingw\lib\libopencv_objdetect2411.dll.a\
D:\yzbx_allthing\build\install\x64\mingw\lib\libopencv_ocl2411.dll.a\
D:\yzbx_allthing\build\install\x64\mingw\lib\libopencv_photo2411.dll.a\
D:\yzbx_allthing\build\install\x64\mingw\lib\libopencv_stitching2411.dll.a\
D:\yzbx_allthing\build\install\x64\mingw\lib\libopencv_superres2411.dll.a\
D:\yzbx_allthing\build\install\x64\mingw\lib\libopencv_ts2411.a\
D:\yzbx_allthing\build\install\x64\mingw\lib\libopencv_video2411.dll.a\
D:\yzbx_allthing\build\install\x64\mingw\lib\libopencv_videostab2411.dll.a
