# external supported variables:
# passed on commandline like "qmake NOWEBKIT=1"
# NOUSEWEBKIT         - disable use of WebKit (Yahoo adapter)
# TRANSDIR_MERKAARTOR - translations directory for merkaartor
# TRANSDIR_SYSTEM     - translations directory for Qt itself
# OUTPUT_DIR          - base directory for local output files
# PREFIX              - base prefix for installation
# NODEBUG             - no debug target
# OSMARENDER          - enable osmarender
# PROJ                - use PROJ4 library for projections
# GDAL    	      - enable GDAL
# MOBILE    	      - enable MOBILE
# GEOIMAGE            - enable geotagged images (needs exiv2)
# GPSD                - use gpsd as location provider
# NVIDIA_HACK         - used to solve nvidia specific slowdown

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
    DEFINES += QT_NO_DEBUG_OUTPUT
    OUTPUT_DIR=$$PWD/binaries/$$(QMAKESPEC)/release
    OBJECTS_DIR += tmp/$$(QMAKESPEC)/obj_release
}

contains(GPSD,1) {
    DEFINES += USEGPSD
}

contains(NVIDIA_HACK,1) {
    DEFINES += ENABLE_NVIDIA_HACK
}

DESTDIR = $$OUTPUT_DIR/bin

INCLUDEPATH += . Render qextserialport GPS NameFinder
DEPENDPATH += . Render qextserialport GPS NameFinder
MOC_DIR = tmp
UI_DIR = tmp
RCC_DIR = tmp

TRANSLATIONS += \
	translations/merkaartor_cs.ts \
	translations/merkaartor_de.ts \
	translations/merkaartor_fr.ts \
	translations/merkaartor_it.ts \
	translations/merkaartor_pl.ts \
	translations/merkaartor_ru.ts

BINTRANSLATIONS += \
	translations/merkaartor_cs.qm \
	translations/merkaartor_de.qm \
	translations/merkaartor_fr.qm \
	translations/merkaartor_it.qm \
	translations/merkaartor_pl.qm \
	translations/merkaartor_ru.qm

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
include(QtStyles/QtStyles.pri)

unix {
    # Prefix: base instalation directory
    isEmpty( PREFIX ) {
		PREFIX = /usr/local
	}
    target.path = $${PREFIX}/bin

    isEmpty(TRANSDIR_MERKAARTOR) {
        TRANSDIR_MERKAARTOR = $${PREFIX}/share/merkaartor/translations
    }
    isEmpty(TRANSDIR_SYSTEM) {
        TRANSDIR_SYSTEM = $${PREFIX}/share/qt4/translations
    }
}

win32 {
	INCLUDEPATH += $$OUTPUT_DIR/include
	LIBS += -L$$OUTPUT_DIR/lib
	RC_FILE = Icons/merkaartor-win32.rc
}

INSTALLS += target

win32-msvc* {
    DEFINES += _USE_MATH_DEFINES
}


translations.path =  $${TRANSDIR_MERKAARTOR}
translations.files = $${BINTRANSLATIONS}
DEFINES += TRANSDIR_MERKAARTOR=\"\\\"$$translations.path\\\"\"
INSTALLS += translations

DEFINES += TRANSDIR_SYSTEM=\"\\\"$${TRANSDIR_SYSTEM}\\\"\"

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

contains (PROJ, 1) {
	DEFINES += USE_PROJ
	LIBS += -lproj
}

contains (GDAL, 1) {
	DEFINES += USE_GDAL
	win32 {
		LIBS += -lgdal_i
		world_shp.path = share/world_shp
	}
	unix {
		LIBS += -lgdal
		world_shp.path = $${PREFIX}/share/merkaartor/world_shp
	}
	
	world_shp.files = \
		share/world_shp/world_adm0.shp \
		share/world_shp/world_adm0.shx \
		share/world_shp/world_adm0.dbf

	DEFINES += WORLD_SHP=\"\\\"$$world_shp.path/world_adm0.shp\\\"\"
	INSTALLS += world_shp
}

#   INCLUDEPATH += binaries/win32-g++/debug/include
#   LIBS += -Lbinaries/win32-g++/debug/lib

#   DESTDIR = binaries/win32-g++/debug/bin


!isEmpty(TRANSLATIONS) {

  isEmpty(QMAKE_LRELEASE) {
    win32:QMAKE_LRELEASE = $$[QT_INSTALL_BINS]\lrelease.exe
    else:QMAKE_LRELEASE = $$[QT_INSTALL_BINS]/lrelease
  }

 TS_DIR = translations

  TSQM.name = lrelease ${QMAKE_FILE_IN}
  TSQM.input = TRANSLATIONS
  TSQM.output = $$TS_DIR/${QMAKE_FILE_BASE}.qm
  TSQM.commands = $$QMAKE_LRELEASE ${QMAKE_FILE_IN}
  TSQM.CONFIG = no_link
  QMAKE_EXTRA_COMPILERS += TSQM
  PRE_TARGETDEPS += compiler_TSQM_make_all

} else:message(No translation files in project)


