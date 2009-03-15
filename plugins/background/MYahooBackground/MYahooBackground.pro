# PREFIX              - base prefix for installation
# NODEBUG             - no debug target

DEFINES += VERSION=\"\\\"$$VERSION\\\"\"
DEFINES += REVISION=\"\\\"$$REVISION\\\"\"

!contains(NODEBUG,1) {
    CONFIG += debug
    OBJECTS_DIR += tmp/$$(QMAKESPEC)/obj_debug
}
contains(NODEBUG,1) {
    CONFIG += release
    DEFINES += NDEBUG
    DEFINES += QT_NO_DEBUG_OUTPUT
    OBJECTS_DIR += tmp/$$(QMAKESPEC)/obj_release
}

TEMPLATE = lib
CONFIG += plugin

QT += core

TARGET = $$qtLibraryTarget(MYahooBackgroundPlugin)
DEPENDPATH += ..
INCLUDEPATH += ..
HEADERS += \
	IMapAdapter.h \
	IImageManager.h \
	mapadapter.h \
	tilemapadapter.h \
	yahoolegalmapadapter.h 

SOURCES += \ 
	mapadapter.cpp \
	tilemapadapter.cpp \
	yahoolegalmapadapter.cpp 

RESOURCES += \
	MYahooBackground.qrc

#DEFINES += QT_NO_DEBUG_OUTPUT

OUTPUT_DIR=$$PWD/../../../binaries
UI_DIR += tmp/$$(QMAKESPEC)
MOC_DIR += tmp/$$(QMAKESPEC)
RCC_DIR += tmp/$$(QMAKESPEC)
DESTDIR = $$OUTPUT_DIR/$$(QMAKESPEC)/bin/plugins/background

unix {
    # Prefix: base instalation directory
    isEmpty( PREFIX ) {
		PREFIX = /usr/local
	}
	target.path = $${PREFIX}/lib/Merkaartor/plugins/background
	INSTALLS += target
}
