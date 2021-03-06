TEMPLATE = app

CONFIG += console
CONFIG -= app_bundle
CONFIG -= qt
CONFIG += c++11
CONFIG += thread


SOURCES += AddressManager.cpp
SOURCES += OperationManager.cpp
SOURCES += main.cpp
SOURCES += Session.cpp
SOURCES += Server.cpp
SOURCES += ../util/Logger.cpp

INCLUDEPATH += $$PWD/../util/

HEADERS += AddressManager.h
HEADERS += Operation.h
HEADERS += OperationManager.h
HEADERS += Session.h
HEADERS += Server.h
HEADERS += LoggerWrapper.h
HEADERS += ../util/Logger.h

DESTDIR = ./bin
OBJECTS_DIR = ./obj
MOC_DIR = ./moc

LIBS += -pthread
LIBS += -lboost_system
