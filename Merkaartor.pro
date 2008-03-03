TEMPLATE = app
TARGET = merkaartor
DESTDIR = ./release
QT += network xml core gui
CONFIG += debug_and_release

INCLUDEPATH += .
DEPENDPATH += .
MOC_DIR += ./GeneratedFiles/release
OBJECTS_DIR += release
UI_DIR += ./GeneratedFiles

#Include file(s)
include(Merkaartor.pri)
include(QMapControl.pri)
include(ImportExport.pri)

