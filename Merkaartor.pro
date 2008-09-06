# external supported variables:
# passed on commandline like "qmake NOWEBKIT=1"
# NOUSEWEBKIT         - disable use of WebKit (Yahoo adapter)
# NOWEBKIT            - disable building of own WebKit
# TRANSDIR_MERKAARTOR - translations directory for merkaartor
# TRANSDIR_SYSTEM     - translations directory for Qt itself
# OUTPUT_DIR          - base directory for local output files
# PREFIX              - base prefix for installation
# NODEBUG             - no debug target
# OSMARENDER          - enable osmarender
# MOBILE              - enable MOBILE
# GEOIMAGE            - enable geotagged images (needs exiv2)
# GPSD                - use gpsd as location provider

TEMPLATE = app
TARGET = merkaartor

QT += svg network xml core gui

count(NODEBUG,0) {
    CONFIG += debug
    OUTPUT_DIR=$$PWD/binaries/debug
    OBJECTS_DIR += tmp/obj_debug
}
count(NODEBUG,1) {
    CONFIG += release
    DEFINES += NDEBUG
    OUTPUT_DIR=$$PWD/binaries/release
    OBJECTS_DIR += tmp/obj_release
}

count(GPSD,1) {
    DEFINES += USEGPSD
}

DESTDIR = $$OUTPUT_DIR/bin

VERSION="0.12"
DEFINES += VERSION=\"\\\"$$VERSION\\\"\"

INCLUDEPATH += . Render qextserialport GPS
DEPENDPATH += . Render qextserialport GPS
MOC_DIR += tmp
UI_DIR += tmp

TRANSLATIONS += \
	merkaartor_cs.ts \
	merkaartor_de.ts \
	merkaartor_cs.ts \
	merkaartor_fr.ts \
	merkaartor_it.ts \
	merkaartor_pl.ts \
	merkaartor_ru.ts

BINTRANSLATIONS += \
	merkaartor_cs.qm \
	merkaartor_de.qm \
	merkaartor_cs.qm \
	merkaartor_fr.qm \
	merkaartor_it.qm \
	merkaartor_pl.qm \
	merkaartor_ru.qm

#Include file(s)
include(Merkaartor.pri)
include(QMapControl.pri)
include(ImportExport.pri)
include(Render/Render.pri)
include(qextserialport/qextserialport.pri)
include(GPS/GPS.pri)

unix {
    target.path = /usr/local/bin
    # Prefix: base instalation directory
    count( PREFIX, 1 ) {
        target.path = $${PREFIX}/bin

        isEmpty(TRANSDIR_MERKAARTOR) {
            TRANSDIR_MERKAARTOR = $${PREFIX}/share/merkaartor/translations
        }
        isEmpty(TRANSDIR_SYSTEM) {
            TRANSDIR_SYSTEM = $${PREFIX}/share/qt4/translations
        }

    }
    INSTALLS += target
}

win32 {
	INCLUDEPATH += $$OUTPUT_DIR/include
	LIBS += -L$$OUTPUT_DIR/lib
    RC_FILE = Icons/merkaartor-win32.rc
}

win32-msvc* {
    DEFINES += _USE_MATH_DEFINES
}

count(TRANSDIR_MERKAARTOR, 1) {
    translations.path =  $${TRANSDIR_MERKAARTOR}
    translations.files = $${BINTRANSLATIONS}
    DEFINES += TRANSDIR_MERKAARTOR=\"\\\"$$translations.path\\\"\"
    INSTALLS += translations
}

count(TRANSDIR_SYSTEM, 1) {
    DEFINES += TRANSDIR_SYSTEM=\"\\\"$${TRANSDIR_SYSTEM}\\\"\"
}

isEmpty(NOUSEWEBKIT) {
    DEFINES += YAHOO
    SOURCES += QMapControl/yahoolegalmapadapter.cpp QMapControl/browserimagemanager.cpp
    HEADERS += QMapControl/yahoolegalmapadapter.h QMapControl/browserimagemanager.h
    isEmpty(NOWEBKIT) {
        include(webkit/WebKit.pri)
    }
    QT += webkit
}

count(MOBILE,1) {
    DEFINES += _MOBILE
    win32-wince* {
        DEFINES += _WINCE
    }
}


count(GEOIMAGE, 1) {
	DEFINES += GEOIMAGE
	LIBS += -lexiv2
	include(GeoImage.pri)
}
