#-------------------------------------------------
#
# Project created by QtCreator 2012-07-24T15:30:11
#
#-------------------------------------------------

QT       += core gui

TARGET = flowercontour
TEMPLATE = app

SOURCES += main.cpp\
        mainwindow.cpp \
    CGrabCut.cpp

HEADERS  += mainwindow.h \
    CGrabCut.h

FORMS    += mainwindow.ui

OpenCV_Lib = C:/OpenCV/opencv243/lib

INCLUDEPATH +=  C:/OpenCV/opencv243/include\
                C:/OpenCV/opencv243/include/opencv2\


Release: LIBS +=    $$OpenCV_Lib/opencv_highgui243.lib\
                    $$OpenCV_Lib/opencv_calib3d243.lib\
                    $$OpenCV_Lib/opencv_contrib243.lib\
                    $$OpenCV_Lib/opencv_core243.lib\
                    $$OpenCV_Lib/opencv_features2d243.lib\
                    $$OpenCV_Lib/opencv_flann243.lib\
                    $$OpenCV_Lib/opencv_gpu243.lib\
                    $$OpenCV_Lib/opencv_imgproc243.lib\
                    $$OpenCV_Lib/opencv_legacy243.lib\
                    $$OpenCV_Lib/opencv_ml243.lib\
                    $$OpenCV_Lib/opencv_objdetect243.lib\
                    $$OpenCV_Lib/opencv_ts243.lib\
                    $$OpenCV_Lib/opencv_video243.lib


