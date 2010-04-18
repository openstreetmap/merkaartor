include(../../common.pri)
include(../common.pri)

QT += network
TARGET = $$qtLibraryTarget(MWalkingPapersBackgroundPlugin)
DEPENDPATH += $${MERKAARTOR_SRC_DIR}/interfaces
INCLUDEPATH += $${MERKAARTOR_SRC_DIR}/interfaces
HEADERS += \
    IMapAdapter.h \
    IImageManager.h \
    WalkingPapersAdapter.h

SOURCES += \
    WalkingPapersAdapter.cpp

COMMON_DIR=$${MERKAARTOR_SRC_DIR}/binaries

win32 {
    INCLUDEPATH += $$COMMON_DIR/include
    LIBS += -L$$COMMON_DIR/lib
}

contains(ZBAR, 1) {
    DEFINES += USE_ZBAR
    LIBS += -lzbar
}
