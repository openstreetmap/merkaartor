include(../../common.pri)
include(../common.pri)

QT += network xml svg
TARGET = $$qtLibraryTarget(MGosmoreBackgroundPlugin)
INCLUDEPATH += $$MERKAARTOR_SRC_DIR/src
DEPENDPATH += $$MERKAARTOR_SRC_DIR/src

INCLUDEPATH += $$MERKAARTOR_SRC_DIR/src/PaintStyle
DEPENDPATH += $$MERKAARTOR_SRC_DIR/src/PaintStyle
include(../../../src/PaintStyle/PaintStyle.pri)

HEADERS += \
    IMapAdapter.h \
    IImageManager.h \
    GosmoreAdapter.h \
    libgosm.h \
    GosmoreFeature.h

SOURCES += \
    Utils/TagSelector.cpp \
    Utils/SvgCache.cpp \
    GosmoreAdapter.cpp \
    libgosm.cpp \
    GosmoreFeature.cpp

COMMON_DIR=$${MERKAARTOR_SRC_DIR}/binaries

win32 {
    INCLUDEPATH += $$COMMON_DIR/include
    LIBS += -L$$COMMON_DIR/lib
}

DEFINES += NOGTK

RESOURCES += \
    MGosmoreBackground.qrc
