TEMPLATE = app
TARGET = ./release/merkaartor
QT += network xml core gui
CONFIG += release

DEFINES += MAJORVERSION="0"
DEFINES += MINORVERSION="11"

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

