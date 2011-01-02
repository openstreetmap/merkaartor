HEADERS += \
    Backend/MemoryBackend.h

SOURCES += \
    Backend/MemoryBackend.cpp

contains (SPATIALITE, 1) {
    HEADERS += \
        Backend/SpatialiteBase.h \
        Backend/SpatialiteBackend.h

    SOURCES += \
        Backend/SpatialiteBase.cpp \
        Backend/SpatialiteBackend.cpp
}
