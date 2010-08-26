include(../../common.pri)
include(../common.pri)
TARGET = $$qtLibraryTarget(MNavitBackgroundPlugin)
QT += network \
    xml \
    svg
INCLUDEPATH += $$MERKAARTOR_SRC_DIR/src
DEPENDPATH += $$MERKAARTOR_SRC_DIR/src
INCLUDEPATH += $$MERKAARTOR_SRC_DIR/src/PaintStyle
DEPENDPATH += $$MERKAARTOR_SRC_DIR/src/PaintStyle
include(../../../src/PaintStyle/PaintStyle.pri)
HEADERS += IMapAdapter.h \
    IImageManager.h \
    NavitAdapter.h \
    NavitBin.h \
    NavitZip.h \
    NavitFeature.h
SOURCES += \
    Utils/TagSelector.cpp \
    Utils/SvgCache.cpp \
    NavitAdapter.cpp \
    NavitBin.cpp \
    NavitZip.cpp \
    NavitFeature.cpp \

COMMON_DIR = $${MERKAARTOR_SRC_DIR}/binaries
win32 {
    INCLUDEPATH += $$COMMON_DIR/include
    LIBS += -L$$COMMON_DIR/lib
}
RESOURCES += MNavitBackground.qrc
