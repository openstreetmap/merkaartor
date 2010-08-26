# -------------------------------------------------
# Project created by QtCreator 2010-08-26T14:46:56
# -------------------------------------------------
QT += svg xml
TARGET = mobilemerk
TEMPLATE = app
INCLUDEPATH += ../src
DEPENDPATH += ../src
DEFINES += _MOBILE
SOURCES += main.cpp \
    MainWindow.cpp \
    MapView.cpp \
    Maps/Coord.cpp \
    Maps/Projection.cpp
HEADERS += MainWindow.h \
    MapView.h \
    Maps/Coord.h \
    Maps/Projection.h
FORMS += MainWindow.ui
