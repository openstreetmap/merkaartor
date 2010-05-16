include(../../common.pri)
include(../common.pri)

QT += network
TARGET = $$qtLibraryTarget(MNavitBackgroundPlugin)
INCLUDEPATH += $$MERKAARTOR_SRC_DIR/src

HEADERS += \
    IMapAdapter.h \
    IImageManager.h \
    NavitAdapter.h \
    NavitBin.h \
    NavitZip.h

SOURCES += \
    NavitAdapter.cpp \
    NavitBin.cpp \
    NavitZip.cpp

COMMON_DIR=$${MERKAARTOR_SRC_DIR}/binaries

win32 {
    INCLUDEPATH += $$COMMON_DIR/include
    LIBS += -L$$COMMON_DIR/lib
}
