TEMPLATE = app
CONFIG += console
CONFIG -= app_bundle
CONFIG -= qt
CONFIG += c++11

SOURCES += main.cpp \
    FileOperations.cpp

HEADERS += \
    FileOperations.h

LIBS += -lboost_system
LIBS += -lboost_filesystem
DESTDIR += ./bin
OBJECTS_DIR = ./obj
MOC_DIR = ./moc

