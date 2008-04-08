TEMPLATE = app
TARGET = ./release/merkaartor
QT += network xml core gui
CONFIG += debug_and_release

DEFINES += MAJORVERSION="0"
DEFINES += MINORVERSION="11"

INCLUDEPATH += .
DEPENDPATH += .
MOC_DIR += ./GeneratedFiles/release
OBJECTS_DIR += release
UI_DIR += ./GeneratedFiles

TRANSLATIONS += \
	merkaartor_de.ts \
	merkaartor_fr.ts

#Include file(s)
include(Merkaartor.pri)
include(QMapControl.pri)
include(ImportExport.pri)

win32-msvc* {
    DEFINES += _USE_MATH_DEFINES
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

