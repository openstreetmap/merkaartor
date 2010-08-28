# -------------------------------------------------
# Project created by QtCreator 2010-08-26T14:46:56
# -------------------------------------------------
QT += svg xml
TARGET = mobilemerk
TEMPLATE = app
DEFINES += _MOBILE

include(src/src.pri)

INCLUDEPATH += thirdparty/MouseMachine
DEPENDPATH += thirdparty/MouseMachine
include(thirdparty/MouseMachine/MouseMachine.pri)

INCLUDEPATH += ../plugins/background/MGosmoreBackground
DEPENDPATH += ../plugins/background/MGosmoreBackground
include(../plugins/background/MGosmoreBackground/MGosmoreBackground.pri)

INCLUDEPATH += ../src/PaintStyle
DEPENDPATH += ../src/PaintStyle
include(../src/PaintStyle/PaintStyle.pri)

INCLUDEPATH += ../interfaces
DEPENDPATH += ../interfaces
