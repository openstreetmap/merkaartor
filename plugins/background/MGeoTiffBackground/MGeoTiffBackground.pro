include(../../common.pri)
include(../common.pri)

TARGET = $$qtLibraryTarget(MGeoTiffBackgroundPlugin)
DEPENDPATH += $${MERKAARTOR_SRC_DIR}/interfaces
INCLUDEPATH += $${MERKAARTOR_SRC_DIR}/interfaces
include ($${MERKAARTOR_SRC_DIR}/interfaces/Interfaces.pri)

DEPENDPATH += $${MERKAARTOR_SRC_DIR}/src/Utils
INCLUDEPATH += $${MERKAARTOR_SRC_DIR}/src/Utils

DEFINES += NO_PREFS

HEADERS += \
    ProjectionChooser.h \
    GeoTiffAdapter.h

SOURCES += \
    ProjectionChooser.cpp \
    GeoTiffAdapter.cpp

FORMS += \
    ProjectionChooser.ui

COMMON_DIR=$${MERKAARTOR_SRC_DIR}/binaries

win32 {
    INCLUDEPATH += $$COMMON_DIR/include
    LIBS += -L$$COMMON_DIR/lib
    win32-msvc*:LIBS += -lgdal_i
    win32-g++:LIBS += -lgdal
}
unix {
    LIBS += $$system(gdal-config --libs)
    QMAKE_CXXFLAGS += $$system(gdal-config --cflags)
    QMAKE_CFLAGS += $$system(gdal-config --cflags)
}
