include(../../common.pri)
include(../common.pri)

TARGET = $$qtLibraryTarget(MGdalBackgroundPlugin)
DEPENDPATH += $${MERKAARTOR_SRC_DIR}/interfaces
INCLUDEPATH += $${MERKAARTOR_SRC_DIR}/interfaces
HEADERS += \
    IMapAdapter.h \
    IImageManager.h \
    GdalAdapter.h

SOURCES += \
    GdalAdapter.cpp

COMMON_DIR=$${MERKAARTOR_SRC_DIR}/binaries

win32 {
    LIBS += -L$$COMMON_DIR/lib
    win32-msvc*:LIBS += -lgdal_i
    win32-g++:LIBS += -lgdal
}
unix {
    INCLUDEPATH += /usr/include/gdal
    LIBS += $$system(gdal-config --libs)
    QMAKE_CXXFLAGS += $$system(gdal-config --cflags)
    QMAKE_CFLAGS += $$system(gdal-config --cflags)
}
