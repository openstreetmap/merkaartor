# external supported variables:
# passed on commandline like "qmake NOWEBKIT=1"
# TRANSDIR_MERKAARTOR - translations directory for merkaartor
# TRANSDIR_SYSTEM     - translations directory for Qt itself
# OUTPUT_DIR          - base directory for local output files
# PREFIX              - base prefix for installation (default: /usr/local)
# LIBDIR              - base directory for plugins (default: $$PREFIX/lib)
# NODEBUG             - no debug target
# OSMARENDER          - enable osmarender
# GDAL    	      - enable GDAL
# MOBILE    	      - enable MOBILE
# GEOIMAGE            - enable geotagged images (needs exiv2)
# GPSD                - use gpsd as location provider
# NVIDIA_HACK         - used to solve nvidia specific slowdown
# FORCE_CUSTOM_STYLE  - force custom style (recommended on Linux until the "expanding dock" is solved upstream)
# THREADED_BROWSERIMAGEMANAGER

#Static config
include (Config.pri)

#Custom config
include(Custom.pri)

#Qt Version
QT_VERSION = $$[QT_VERSION]
QT_VERSION = $$split(QT_VERSION, ".")
QT_VER_MAJ = $$member(QT_VERSION, 0)
QT_VER_MIN = $$member(QT_VERSION, 1)

lessThan(QT_VER_MAJ, 4) | lessThan(QT_VER_MIN, 4) {
    error(Merkaartor requires Qt 4.4 or newer but Qt $$[QT_VERSION] was detected.)
}
DEFINES += VERSION=$$VERSION
DEFINES += REVISION=$$REVISION

TEMPLATE = app
TARGET = merkaartor

CONFIG += rtti stl 
#CONFIG -= exceptions
QT += svg network xml core gui

!contains(NODEBUG,1) {
    CONFIG += debug
    #OBJECTS_DIR += $$PWD/../tmp/$$(QMAKESPEC)/obj_debug
}
contains(NODEBUG,1) {
    CONFIG += release
    DEFINES += NDEBUG
    #OBJECTS_DIR += $$PWD/../tmp/$$(QMAKESPEC)/obj_release
}
COMMON_DIR=$$PWD/../binaries
OUTPUT_DIR=$$PWD/../binaries/$$(QMAKESPEC)
DESTDIR = $$OUTPUT_DIR/bin

#UI_DIR += $$PWD/../tmp/$$(QMAKESPEC)
#MOC_DIR += $$PWD/../tmp/$$(QMAKESPEC)
#RCC_DIR += $$PWD/../tmp/$$(QMAKESPEC)

INCLUDEPATH += $$PWD/../include $$PWD/../interfaces
DEPENDPATH += $$PWD/../interfaces

win32 {
	INCLUDEPATH += $$COMMON_DIR/include
	LIBS += -L$$COMMON_DIR/lib
	RC_FILE = $$PWD/../Icons/merkaartor-win32.rc
}

contains(GPSD,1) {
    DEFINES += USEGPSD
}

contains(FORCE_CUSTOM_STYLE,1) {
    DEFINES += FORCED_CUSTOM_STYLE
}

contains(NVIDIA_HACK,1) {
    DEFINES += ENABLE_NVIDIA_HACK
}

INCLUDEPATH += $$PWD Render qextserialport GPS NameFinder
DEPENDPATH += $$PWD Render qextserialport GPS NameFinder

!contains(NOUSEWEBKIT,1) {
	greaterThan(QT_VER_MAJ, 3) : greaterThan(QT_VER_MIN, 3) {
    	DEFINES += USE_WEBKIT
    	SOURCES += QMapControl/browserimagemanager.cpp
    	HEADERS += QMapControl/browserimagemanager.h
    	QT += webkit
        contains(THREADED_BROWSERIMAGEMANAGER,1): DEFINES += BROWSERIMAGEMANAGER_IS_THREADED
	}
}

TRANSLATIONS += \
	../translations/merkaartor_ar.ts \
	../translations/merkaartor_cs.ts \
	../translations/merkaartor_de.ts \
	../translations/merkaartor_es.ts \
	../translations/merkaartor_fr.ts \
	../translations/merkaartor_it.ts \
	../translations/merkaartor_ja.ts \
	../translations/merkaartor_nl.ts \
	../translations/merkaartor_pl.ts \
	../translations/merkaartor_ru.ts

BINTRANSLATIONS += \
	../translations/merkaartor_ar.qm \
	../translations/merkaartor_cs.qm \
	../translations/merkaartor_de.qm \
	../translations/merkaartor_es.qm \
	../translations/merkaartor_fr.qm \
	../translations/merkaartor_it.qm \
	../translations/merkaartor_ja.qm \
	../translations/merkaartor_nl.qm \
	../translations/merkaartor_pl.qm \
	../translations/merkaartor_ru.qm

#Include file(s)
include(Merkaartor.pri)
include(QMapControl.pri)
include(ImportExport.pri)
include(Render/Render.pri)
!symbian:include(qextserialport/qextserialport.pri)
include(GPS/GPS.pri)
include(Tools/Tools.pri)
include(TagTemplate/TagTemplate.pri)
include(NameFinder/NameFinder.pri)


!win32 {
    # Prefix: base instalation directory
    isEmpty( PREFIX ) {
		PREFIX = /usr/local
	}
    isEmpty( LIBDIR ) {
		LIBDIR = $${PREFIX}/lib
	}
	DEFINES += PLUGINS_DIR=$${LIBDIR}/merkaartor/plugins
    target.path = $${PREFIX}/bin
    SHARE_DIR = $${PREFIX}/share/merkaartor

    isEmpty(TRANSDIR_MERKAARTOR) {
        TRANSDIR_MERKAARTOR = $${SHARE_DIR}/translations
    }
}
win32 {
	DEFINES += PLUGINS_DIR=plugins
    SHARE_DIR = share
    isEmpty(TRANSDIR_MERKAARTOR) {
        TRANSDIR_MERKAARTOR = translations
    }
    isEmpty(TRANSDIR_SYSTEM) {
        TRANSDIR_SYSTEM = translations
    }
}

DEFINES += SHARE_DIR=$${SHARE_DIR}
INSTALLS += target

win32-msvc* {
    DEFINES += _USE_MATH_DEFINES
}


count(TRANSDIR_MERKAARTOR, 1) {
	translations.path =  $${TRANSDIR_MERKAARTOR}
	translations.files = $${BINTRANSLATIONS}
	DEFINES += TRANSDIR_MERKAARTOR=$$translations.path
	INSTALLS += translations
}

count(TRANSDIR_SYSTEM, 1) {
	DEFINES += TRANSDIR_SYSTEM=$${TRANSDIR_SYSTEM}
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

lists.path = $${SHARE_DIR}
lists.files = \ 
    $$PWD/../share/BookmarksList.xml \
    $$PWD/../share/Projections.xml \
    $$PWD/../share/WmsServersList.xml \
    $$PWD/../share/TmsServersList.xml
INSTALLS += lists

contains (GDAL, 1) {
	DEFINES += USE_GDAL
	win32 {
		win32-msvc*:LIBS += -lgdal_i
		win32-g++:LIBS += -lgdal
	}
	unix {
		INCLUDEPATH += /usr/include/gdal
		LIBS += -lgdal
	}
}
world_shp.path = $${SHARE_DIR}
world_shp.files = $$PWD/../share/world_background.osb

DEFINES += WORLD_SHP=$$world_shp.path/world_background.osb
INSTALLS += world_shp

desktop.path = $${PREFIX}/share/applications
desktop.files = merkaartor.desktop
desktopicons.path = $${PREFIX}/share/icons/hicolor/48x48/apps/
desktopicons.files = $$PWD/../Icons/48x48/merkaartor.png
INSTALLS += desktop desktopicons


