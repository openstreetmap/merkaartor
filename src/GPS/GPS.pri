
FORMS += \
           qgpsmainwindowui.ui

HEADERS += qgps.h \
           qgpssatellitetracker.h \
           qgpsdevice.h \
       SatelliteStrengthView.h

SOURCES += qgps.cpp \
           qgpssatellitetracker.cpp \
           qgpsdevice.cpp \
       SatelliteStrengthView.cpp

DEFINES += USE_GPS

symbian {
    DEPENDPATH += ../S60extensions/location/src
    INCLUDEPATH += ../S60extensions/location/src

    # Mobile extensions headers and sources
    HEADERS += xqlocation.h

    SOURCES += xqlocation.cpp

    HEADERS += xqlocation_s60_p.h
    SOURCES += xqlocation_s60_p.cpp

    LIBS += -llbs
}

contains(GPSDLIB,1) {
    DEFINES += USE_GPSD_LIB
    win32 {
        LIBS += -lQgpsmm
    }

    unix|macx {
        LIBS += -lgps
    }
}
