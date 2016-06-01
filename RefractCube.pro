QT += gui core

CONFIG += c++11

TARGET = RefractCube
CONFIG += console
CONFIG -= app_bundle

TEMPLATE = app

SOURCES += main.cpp \
    refractcube.cpp \
    teapot.cpp \
    skybox.cpp

HEADERS += \
    refractcube.h \
    teapotdata.h \
    teapot.h \
    skybox.h
	
OTHER_FILES += \
    fshader.txt \
    vshader_skybox.txt \
    vshader_object.txt

RESOURCES += \
    shaders.qrc

DISTFILES += \
    fshader.txt \
    vshader_skybox.txt \
    vshader_object.txt
