# PREFIX              - base prefix for installation
# NODEBUG             - no debug target
MERKAARTOR_SRC_DIR = $$PWD/../../..

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
DEPENDPATH += $${MERKAARTOR_SRC_DIR}/interfaces
INCLUDEPATH += $${MERKAARTOR_SRC_DIR}/interfaces
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

OUTPUT_DIR=../../../binaries
UI_DIR += tmp/$$(QMAKESPEC)
MOC_DIR += tmp/$$(QMAKESPEC)
RCC_DIR += tmp/$$(QMAKESPEC)
DESTDIR = $$OUTPUT_DIR/$$(QMAKESPEC)/bin/plugins/background

target.path = $${PREFIX}/lib/Merkaartor/plugins/background
INSTALLS += target
