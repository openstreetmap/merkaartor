TEMPLATE = app
TARGET = ./release/merkaartor
QT += network xml core gui
CONFIG += debug_and_release

VERSION="0.11"

DEFINES += VERSION=\"\\\"$$VERSION\\\"\"

INCLUDEPATH += .
DEPENDPATH += .
MOC_DIR += ./GeneratedFiles/release
OBJECTS_DIR += release
UI_DIR += ./GeneratedFiles

TRANSLATIONS += \
	merkaartor_de.ts \
	merkaartor_fr.ts

BINTRANSLATIONS += \
	merkaartor_de.qm \
	merkaartor_fr.qm

#Include file(s)
include(Merkaartor.pri)
include(QMapControl.pri)
include(ImportExport.pri)

unix {
    # Prefix: base instalation directory
    count( PREFIX, 1 ) {
        target.path = $${PREFIX}/bin
        INSTALLS += target

        isEmpty(TRANSDIR_MERKAARTOR) {
            TRANSDIR_MERKAARTOR = $${PREFIX}/share/merkaartor/translations
        }
        isEmpty(TRANSDIR_SYSTEM) {
            TRANSDIR_SYSTEM = $${PREFIX}/share/qt4/translations
        }

    }
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

osmarender {
    !win32-g++ {
        include(Render.pri)
    }
}

yahoo {
    unix {
        WEBKIT_SRC_DIR = "/var/src/WebKit"
    }
    win32 {
        WEBKIT_SRC_DIR = "C:/home/cbrowet/Programming/merkaartor_webkit"
    }

    DEFINES += YAHOO
    SOURCES += QMapControl/yahoolegalmapadapter.cpp QMapControl/browserimagemanager.cpp
    HEADERS += QMapControl/yahoolegalmapadapter.h QMapControl/browserimagemanager.h

    lessThan(QT_MINOR_VERSION, 4) {
		# NOTE: LD_LIBRARY_PATH must be set to $$WEBKIT_SRC_DIR/WebKitBuild/Release/lib prior to execution if not in ld.so-conf
        INCLUDEPATH += $$WEBKIT_SRC_DIR/WebKit/qt/Api
        LIBS += -L$$WEBKIT_SRC_DIR/WebKitBuild/Release/lib -lQtWebKit
    }
}

