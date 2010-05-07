include(../../common.pri)
include(../common.pri)

QT += network
TARGET = $$qtLibraryTarget(MGosmoreBackgroundPlugin)
INCLUDEPATH += $$MERKAARTOR_SRC_DIR/src

HEADERS += \
    IMapAdapter.h \
    IImageManager.h \
    GosmoreAdapter.h \
    libgosm.h

SOURCES += \
    GosmoreAdapter.cpp \
    libgosm.cpp

COMMON_DIR=$${MERKAARTOR_SRC_DIR}/binaries

win32 {
    INCLUDEPATH += $$COMMON_DIR/include
    LIBS += -L$$COMMON_DIR/lib
}

DEFINES += NOGTK
