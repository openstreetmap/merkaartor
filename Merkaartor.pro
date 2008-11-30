# external supported variables:
# passed on commandline like "qmake NOWEBKIT=1"
# NOUSEWEBKIT         - disable use of WebKit (Yahoo adapter)
# TRANSDIR_MERKAARTOR - translations directory for merkaartor
# TRANSDIR_SYSTEM     - translations directory for Qt itself
# OUTPUT_DIR          - base directory for local output files
# PREFIX              - base prefix for installation
# NODEBUG             - no debug target
# OSMARENDER          - enable osmarender
# GDAL    	      - enable GDAL
# MOBILE    	      - enable MOBILE
# GEOIMAGE            - enable geotagged images (needs exiv2)
# GPSD                - use gpsd as location provider

#Static config
include (Config.pri)

#Custom config
include(Custom.pri)

#Qt Version
QT_VERSION = $$[QT_VERSION]
QT_VERSION = $$split(QT_VERSION, ".")
QT_VER_MAJ = $$member(QT_VERSION, 0)
QT_VER_MIN = $$member(QT_VERSION, 1)

lessThan(QT_VER_MAJ, 4) | lessThan(QT_VER_MIN, 3) {
    error(Merkaartor requires Qt 4.3 or newer but Qt $$[QT_VERSION] was detected.)
}
DEFINES += VERSION=\"\\\"$$VERSION\\\"\"
DEFINES += REVISION=\"\\\"$$REVISION\\\"\"

TEMPLATE = app
TARGET = merkaartor

QT += svg network xml core gui

!contains(NODEBUG,1) {
    CONFIG += debug
    OUTPUT_DIR=$$PWD/binaries/$$(QMAKESPEC)/debug
    OBJECTS_DIR += tmp/$$(QMAKESPEC)/obj_debug
}
contains(NODEBUG,1) {
    CONFIG += release
    DEFINES += NDEBUG
    OUTPUT_DIR=$$PWD/binaries/$$(QMAKESPEC)/release
    OBJECTS_DIR += tmp/$$(QMAKESPEC)/obj_release
}

contains(GPSD,1) {
    DEFINES += USEGPSD
}

DESTDIR = $$OUTPUT_DIR/bin


INCLUDEPATH += . Render qextserialport GPS NameFinder
DEPENDPATH += . Render qextserialport GPS NameFinder
MOC_DIR = tmp
UI_DIR = tmp
RCC_DIR = tmp

TRANSLATIONS += \
	merkaartor_cs.ts \
	merkaartor_de.ts \
	merkaartor_fr.ts \
	merkaartor_it.ts \
	merkaartor_pl.ts \
	merkaartor_ru.ts

BINTRANSLATIONS += \
	merkaartor_cs.qm \
	merkaartor_de.qm \
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
include(Tools/Tools.pri)
include(TagTemplate/TagTemplate.pri)
include(NameFinder/NameFinder.pri)

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
   greaterThan(QT_VER_MAJ, 3) : greaterThan(QT_VER_MIN, 3) {
        DEFINES += YAHOO
        SOURCES += QMapControl/yahoolegalmapadapter.cpp QMapControl/browserimagemanager.cpp
        HEADERS += QMapControl/yahoolegalmapadapter.h QMapControl/browserimagemanager.h
        QT += webkit
    }
}

contains(MOBILE,1) {
    DEFINES += _MOBILE
    win32-wince* {
      DEFINES += _WINCE
    }
}

contains(GEOIMAGE, 1) {
	DEFINES += GEOIMAGE
	LIBS += -lexiv2
	include(GeoImage.pri)
}


