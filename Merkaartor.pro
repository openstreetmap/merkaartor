TEMPLATE = app
TARGET = merkaartor
QT += network xml core gui
CONFIG += debug osmarender

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

