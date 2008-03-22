TEMPLATE = app
TARGET = merkaartor
DESTDIR = ./release
QT += network xml core gui
CONFIG += debug_and_release yahoo google osmarender

DEFINES += MAJORVERSION="0"
DEFINES += MINORVERSION="10"

INCLUDEPATH += .
DEPENDPATH += .
MOC_DIR += ./GeneratedFiles/release
OBJECTS_DIR += release
UI_DIR += ./GeneratedFiles

#Include file(s)
include(Merkaartor.pri)
include(QMapControl.pri)
include(ImportExport.pri)

win32-msvc* {
    DEFINES += _USE_MATH_DEFINES
}

osmarender {
    !win32-g++ {
        include(Render.pri)
    }
}

yahoo {
    DEFINES += yahoo_illegal
    SOURCES += QMapControl/yahoomapadapter.cpp
    HEADERS += QMapControl/yahoomapadapter.h
}

google {
    DEFINES += google_illegal
    SOURCES += QMapControl/googlesatmapadapter.cpp
    HEADERS += QMapControl/googlesatmapadapter.h
}
