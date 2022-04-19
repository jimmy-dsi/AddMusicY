TEMPLATE = app
CONFIG += console
CONFIG -= app_bundle
CONFIG -= qt

QMAKE_LFLAGS += -static-libgcc
QMAKE_CXXFLAGS += -std=c++11

SOURCES += main.cpp \
    brr.cpp \
    mml.cpp \
    music.cpp \
    sample.cpp \
    macros.cpp

HEADERS += \
    music.h \
    byte.h \
    mml.h \
    macros.h \
    sample.h \
    brr.h \
    global.h


