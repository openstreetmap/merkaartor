INCLUDEPATH += $$MERKAARTOR_SRC_DIR/GPS
DEPENDPATH += $$MERKAARTOR_SRC_DIR/GPS

FORMS += \
           qgpsmainwindowui.ui

HEADERS += qgps.h \
           qgpssatellitetracker.h \
           qgpsdevice.h \
           SatelliteStrengthView.h

SOURCES += qgps.cpp \
           qgpssatellitetracker.cpp \
           qgpsdevice.cpp \
           SatelliteStrengthView.cpp \

DEFINES += USE_GPS

contains(GPSDLIB,1) {
    DEFINES += USE_GPSD_LIB
    win32 {
        LIBS += -lQgpsmm
    }

    unix|macx {
        LIBS += -lgps
    }
}

!isEmpty(MOBILE)
{
  CONFIG += mobility
  MOBILITY += location

#  HEADERS += \
#    GpsFix.h

#  SOURCES +=  \
#    GpsFix.cpp
}
