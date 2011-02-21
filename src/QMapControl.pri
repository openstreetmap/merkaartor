INCLUDEPATH += $$MERKAARTOR_SRC_DIR/QMapControl
DEPENDPATH += $$MERKAARTOR_SRC_DIR/QMapControl

# Input
HEADERS += \
           imagemanager.h \
           mapadapter.h \
           mapnetwork.h \
           wmsmapadapter.h \
           WmscMapAdapter.h \
           tilemapadapter.h

SOURCES += \
           IImageManager.cpp \
           imagemanager.cpp \
           mapadapter.cpp \
           mapnetwork.cpp \
           wmsmapadapter.cpp \
           WmscMapAdapter.cpp \
           tilemapadapter.cpp

QT += network

!contains(NOUSEWEBKIT,1) {
    greaterThan(QT_VER_MAJ, 3) : greaterThan(QT_VER_MIN, 3) {
        DEFINES += USE_WEBKIT
        SOURCES += browserimagemanager.cpp
        HEADERS += browserimagemanager.h
        QT += webkit
        contains(THREADED_BROWSERIMAGEMANAGER,1): DEFINES += BROWSERIMAGEMANAGER_IS_THREADED
    }
}

