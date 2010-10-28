#-------------------------------------------------
#
# Project created by QtCreator 2010-10-18T15:40:26
#
#-------------------------------------------------

QT       += core gui network svg opengl

TARGET = VectorCadastre
TEMPLATE = app

INCLUDEPATH += c:/mingw/include

SOURCES += main.cpp\
    cadastrewrapper.cpp \
    graphicproducer.cpp \
    clippedpathitem.cpp \
    pdfgraphicview.cpp \
    cadastrebrowser.cpp \
    cadastredownloaddialog.cpp

HEADERS  += \
    cadastrewrapper.h \
    graphicproducer.h \
    clippedpathitem.h \
    pdfgraphicview.h \
    cadastrebrowser.h \
    cadastredownloaddialog.h

FORMS    += \
    cadastrebrowser.ui \
    cadastredownloaddialog.ui

LIBS += -Lc:/mingw/bin -lpodofo
