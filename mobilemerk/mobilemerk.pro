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

contains(GOSMORE,1) {
    DEFINES += USE_GOSMORE
    INCLUDEPATH += ../plugins/background/MGosmoreBackground
    DEPENDPATH += ../plugins/background/MGosmoreBackground
    include(../plugins/background/MGosmoreBackground/MGosmoreBackground.pri)
}

contains(NAVIT,1) {
    DEFINES += USE_NAVIT
    INCLUDEPATH += ../plugins/background/MNavitBackground
    DEPENDPATH += ../plugins/background/MNavitBackground
    include(../plugins/background/MNavitBackground/MNavitBackground.pri)
}

INCLUDEPATH += ../src/PaintStyle
DEPENDPATH += ../src/PaintStyle
include(../src/PaintStyle/PaintStyle.pri)

INCLUDEPATH += ../interfaces
DEPENDPATH += ../interfaces
