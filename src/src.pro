# see http://merkaartor.be/wiki/merkaartor/Compiling

#Static config
include (Config.pri)

#Custom config
include(Custom.pri)

CONFIG += debug_and_release c++11
# avoid deprecation warnings which 5.15 introduced.
DEFINES += QT_NO_DEPRECATED_WARNINGS

# This is a workaround to get qDebug() to stdout on Windows. Uncomment if needed.
win32 {
#	CONFIG += console
}

isEmpty(SYSTEM_QTSA) {
  include(../3rdparty/qtsingleapplication-2.6_1-opensource/src/qtsingleapplication.pri)
} else {
  CONFIG += qtsingleapplication
}

#Qt Version
QT_VERSION = $$[QT_VERSION]
QT_VERSION = $$split(QT_VERSION, ".")
QT_VER_MAJ = $$member(QT_VERSION, 0)
QT_VER_MIN = $$member(QT_VERSION, 1)

DEFINES += VERSION=$$VERSION
DEFINES += REVISION=$$REVISION

contains(PORTABLE,1): DEFINES += PORTABLE_BUILD

TEMPLATE = app

CONFIG += rtti stl exceptions
#CONFIG -= exceptions
QT += svg network xml core gui concurrent printsupport
win32-msvc* {
    LIBS += -lzlib
} else {
    LIBS += -lz
}

contains(FRISIUS,1) {
    TARGET = frisius
    DEFINES += FRISIUS_BUILD
    DEFINES += PRODUCT=Frisius
    win32 {
       RC_FILE = $$PWD/../Icons/frisius-win32.rc
    }
} else {
    TARGET = merkaartor
    DEFINES += PRODUCT=Merkaartor
    win32 {
       RC_FILE = $$PWD/../Icons/merkaartor-win32.rc
    }
}

MERKAARTOR_SRC_DIR = $$PWD
COMMON_DIR = $$OUT_PWD/../binaries
OUTPUT_DIR = $$OUT_PWD/../binaries
DESTDIR = $$OUTPUT_DIR/bin

#UI_DIR += $$PWD/../tmp/$$(QMAKESPEC)
#MOC_DIR += $$PWD/../tmp/$$(QMAKESPEC)
#RCC_DIR += $$PWD/../tmp/$$(QMAKESPEC)

INCLUDEPATH += $$PWD $$PWD/../include $$PWD/../interfaces $$MOC_DIR
DEPENDPATH += $$PWD $$PWD/../interfaces

win32 {
    INCLUDEPATH += $$COMMON_DIR/include
    LIBS += -L$$COMMON_DIR/lib
}

macx {
    RC_FILE = $$PWD/../Icons/merkaartor.icns
    QMAKE_INFO_PLIST = $$PWD/../macos/Info.plist
    # This is where we get the the ports from
    INCLUDEPATH += /opt/local/include
    # Stuff from homebrew comes from here
    INCLUDEPATH += /usr/local/include
    LIBS += -L/usr/local/lib
}

contains(NVIDIA_HACK,1) {
    DEFINES += ENABLE_NVIDIA_HACK
}

#Include file(s)
include(common/common.pri)
include(Backend/Backend.pri)
include(../interfaces/Interfaces.pri)
include(PaintStyle/PaintStyle.pri)
include(PaintStyle/PaintStyleEditor.pri)
include(Features/Features.pri)
include(Layers/Layers.pri)
include(Preferences/Preferences.pri)
include(Sync/Sync.pri)
include(Commands/Commands.pri)
include(Interactions/Interactions.pri)
include (Docks/Docks.pri)
include(QMapControl.pri)
include(ImportExport/ImportExport.pri)
include(Render/Render.pri)
!symbian:include(qextserialport/qextserialport.pri)
include(GPS/GPS.pri)
include(Tools/Tools.pri)
include(TagTemplate/TagTemplate.pri)
include(NameFinder/NameFinder.pri)
include(Utils/Utils.pri)
include(QToolBarDialog/QToolBarDialog.pri)

VPATH += $$INCLUDEPATH

# Header files
HEADERS += \
    MainWindow.h

SOURCES += \
    Main.cpp \
    MainWindow.cpp

# Forms
FORMS += \
    MainWindow.ui

OTHER_FILES += ../CHANGELOG ../LICENSE

macx {
    # Prefix: base instalation directory (fixed for the mac)
    PREFIX = /Applications
    LIBDIR = $${PREFIX}/lib${LIB_SUFFIX}
    DEFINES += PLUGINS_DIR=$${PREFIX}/merkaartor.app/Contents/plugins
    target.path = $${PREFIX}
    SHARE_DIR = $${PREFIX}/merkaartor.app/Contents/Resources
    TRANSDIR_MERKAARTOR = $${SHARE_DIR}/
}

unix:!macx {
    # Prefix: base instalation directory
    isEmpty( PREFIX ) {
        PREFIX = /usr/local
    }
    isEmpty( LIBDIR ) {
        LIBDIR = $${PREFIX}/lib${LIB_SUFFIX}
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

TRANSLATIONS += \
    ../translations/merkaartor_cs.ts \
    ../translations/merkaartor_de.ts \
    ../translations/merkaartor_en.ts \
    ../translations/merkaartor_es.ts \
    ../translations/merkaartor_fi.ts \
    ../translations/merkaartor_fr.ts \
    ../translations/merkaartor_hr.ts \
    ../translations/merkaartor_hu.ts \
    ../translations/merkaartor_id_ID.ts \
    ../translations/merkaartor_it.ts \
    ../translations/merkaartor_ja.ts \
    ../translations/merkaartor_nl.ts \
    ../translations/merkaartor_pl.ts \
    ../translations/merkaartor_pt_BR.ts \
    ../translations/merkaartor_ru.ts \
    ../translations/merkaartor_sv.ts \
    ../translations/merkaartor_uk.ts \
    ../translations/merkaartor_zh_TW.ts \


BINTRANSLATIONS += \
    ../translations/merkaartor_cs.qm \
    ../translations/merkaartor_de.qm \
    ../translations/merkaartor_en.qm \
    ../translations/merkaartor_es.qm \
    ../translations/merkaartor_fi.qm \
    ../translations/merkaartor_fr.qm \
    ../translations/merkaartor_hr.qm \
    ../translations/merkaartor_hu.qm \
    ../translations/merkaartor_id_ID.qm \
    ../translations/merkaartor_it.qm \
    ../translations/merkaartor_ja.qm \
    ../translations/merkaartor_nl.qm \
    ../translations/merkaartor_pl.qm \
    ../translations/merkaartor_pt_BR.qm \
    ../translations/merkaartor_ru.qm \
    ../translations/merkaartor_sv.qm \
    ../translations/merkaartor_uk.qm \
    ../translations/merkaartor_zh_TW.qm \

translations.path =  $${TRANSDIR_MERKAARTOR}
translations.files = $${BINTRANSLATIONS}
DEFINES += TRANSDIR_MERKAARTOR=$$translations.path
INSTALLS += translations

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
    include(Docks/GeoImage.pri)
}

lists.path = $${SHARE_DIR}
lists.files = \
    $$PWD/../share/BookmarksList.xml \
    $$PWD/../share/Projections.xml \
    $$PWD/../share/WmsServersList.xml \
    $$PWD/../share/TmsServersList.xml
INSTALLS += lists

win32 {
    win32-msvc*:LIBS += -lgdal_i
    win32-g++:LIBS += -lgdal
}
unix {
    LIBS += $$system(gdal-config --libs)
    QMAKE_CXXFLAGS += $$system(gdal-config --cflags)
    QMAKE_CFLAGS += $$system(gdal-config --cflags)
}

# Setup the PROJ.4
LIBS += -lproj
PKGCONFIG += proj

contains (PROTOBUF, 1) {
    DEFINES += USE_PROTOBUF
}

contains(SANITIZE, 1) {
    SANITIZE=address,undefined
}

contains(SANITIZE, 2) {
    SANITIZE=thread
}

!isEmpty(SANITIZE) {
    QMAKE_CXXFLAGS+=-fsanitize=$${SANITIZE} -fno-omit-frame-pointer
    QMAKE_CFLAGS+=-fsanitize=$${SANITIZE} -fno-omit-frame-pointer
    QMAKE_LFLAGS+=-fsanitize=$${SANITIZE}
}

unix:!macx {
    appdata.path = $${PREFIX}/share/metainfo
    appdata.files = org.merkaartor.merkaartor.appdata.xml
    desktop.path = $${PREFIX}/share/applications
    desktop.files = org.merkaartor.merkaartor.desktop 
    desktopicon8x8.path = $${PREFIX}/share/icons/hicolor/8x8/apps/
    desktopicon8x8.files = $$PWD/../Icons/8x8/merkaartor.png
    desktopicon16x16.path = $${PREFIX}/share/icons/hicolor/16x16/apps/
    desktopicon16x16.files = $$PWD/../Icons/16x16/merkaartor.png
    desktopicon22x22.path = $${PREFIX}/share/icons/hicolor/22x22/apps/
    desktopicon22x22.files = $$PWD/../Icons/22x22/merkaartor.png
    desktopicon24x24.path = $${PREFIX}/share/icons/hicolor/24x24/apps/
    desktopicon24x24.files = $$PWD/../Icons/24x24/merkaartor.png
    desktopicon32x32.path = $${PREFIX}/share/icons/hicolor/32x32/apps/
    desktopicon32x32.files = $$PWD/../Icons/32x32/merkaartor.png
    desktopicon36x36.path = $${PREFIX}/share/icons/hicolor/36x36/apps/
    desktopicon36x36.files = $$PWD/../Icons/36x36/merkaartor.png
    desktopicon40x40.path = $${PREFIX}/share/icons/hicolor/40x40/apps/
    desktopicon40x40.files = $$PWD/../Icons/40x40/merkaartor.png
    desktopicon42x42.path = $${PREFIX}/share/icons/hicolor/42x42/apps/
    desktopicon42x42.files = $$PWD/../Icons/42x42/merkaartor.png
    desktopicon48x48.path = $${PREFIX}/share/icons/hicolor/48x48/apps/
    desktopicon48x48.files = $$PWD/../Icons/48x48/merkaartor.png
    desktopicon64x64.path = $${PREFIX}/share/icons/hicolor/64x64/apps/
    desktopicon64x64.files = $$PWD/../Icons/64x64/merkaartor.png
    desktopicon72x72.path = $${PREFIX}/share/icons/hicolor/72x72/apps/
    desktopicon72x72.files = $$PWD/../Icons/72x72/merkaartor.png
    desktopicon80x80.path = $${PREFIX}/share/icons/hicolor/80x80/apps/
    desktopicon80x80.files = $$PWD/../Icons/80x80/merkaartor.png
    desktopicon96x96.path = $${PREFIX}/share/icons/hicolor/96x96/apps/
    desktopicon96x96.files = $$PWD/../Icons/96x96/merkaartor.png
    desktopicon128x128.path = $${PREFIX}/share/icons/hicolor/128x128/apps/
    desktopicon128x128.files = $$PWD/../Icons/128x128/merkaartor.png
    desktopicon192x192.path = $${PREFIX}/share/icons/hicolor/192x192/apps/
    desktopicon192x192.files = $$PWD/../Icons/192x192/merkaartor.png
    desktopicon256x256.path = $${PREFIX}/share/icons/hicolor/256x256/apps/
    desktopicon256x256.files = $$PWD/../Icons/256x256/merkaartor.png
    desktopicon512x512.path = $${PREFIX}/share/icons/hicolor/512x512/apps/
    desktopicon512x512.files = $$PWD/../Icons/512x512/merkaartor.png
    INSTALLS += appdata \
                desktop \
                desktopicon8x8 \
                desktopicon16x16 \
                desktopicon22x22 \
                desktopicon24x24 \
                desktopicon32x32 \
                desktopicon36x36 \
                desktopicon40x40 \
                desktopicon42x42 \
                desktopicon48x48 \
                desktopicon64x64 \
                desktopicon72x72 \
                desktopicon80x80 \
                desktopicon96x96 \
                desktopicon128x128 \
                desktopicon192x192 \
                desktopicon256x256 \
                desktopicon512x512
}


