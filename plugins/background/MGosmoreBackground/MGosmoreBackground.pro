include(../../common.pri)
include(../common.pri)

QT += network xml svg
TARGET = $$qtLibraryTarget(MGosmoreBackgroundPlugin)
INCLUDEPATH += $$MERKAARTOR_SRC_DIR/src
DEPENDPATH += $$MERKAARTOR_SRC_DIR/src

INCLUDEPATH += $$MERKAARTOR_SRC_DIR/src/PaintStyle $$MERKAARTOR_SRC_DIR/src/Utils
DEPENDPATH += $$MERKAARTOR_SRC_DIR/src/PaintStyle $$MERKAARTOR_SRC_DIR/src/Utils
include(../../../src/PaintStyle/PaintStyle.pri)

include(MGosmoreBackground.pri)

COMMON_DIR=$${MERKAARTOR_SRC_DIR}/binaries

win32 {
    INCLUDEPATH += $$COMMON_DIR/include
    LIBS += -L$$COMMON_DIR/lib
}

