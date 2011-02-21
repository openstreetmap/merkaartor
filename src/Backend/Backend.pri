INCLUDEPATH += $$MERKAARTOR_SRC_DIR/Backend
DEPENDPATH += $$MERKAARTOR_SRC_DIR/Backend

HEADERS += \
    MemoryBackend.h

SOURCES += \
    MemoryBackend.cpp

contains (SPATIALITE, 1) {
    HEADERS += \
        SpatialiteBase.h \
        SpatialiteBackend.h

    SOURCES += \
        SpatialiteBase.cpp \
        SpatialiteBackend.cpp
}
