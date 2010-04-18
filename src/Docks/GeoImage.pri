DEFINES += GEOIMAGE

# Header files
HEADERS += GeoImageDock.h

# Source files
SOURCES += GeoImageDock.cpp
LIBS += -lexiv2
FORMS += PhotoLoadErrorDialog.ui

contains(ZBAR, 1) {
    DEFINES += USE_ZBAR
    LIBS += -lzbar
}

