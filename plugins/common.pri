# PREFIX              - base prefix for installation
# NODEBUG             - no debug target
MERKAARTOR_SRC_DIR = $$PWD/..

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

win32-msvc* {
    DEFINES += _USE_MATH_DEFINES
}

TEMPLATE = lib
CONFIG += plugin

QT += core

#DEFINES += QT_NO_DEBUG_OUTPUT

OUTPUT_DIR=../../../binaries
UI_DIR += tmp/$$(QMAKESPEC)
MOC_DIR += tmp/$$(QMAKESPEC)
RCC_DIR += tmp/$$(QMAKESPEC)
DESTDIR = $$OUTPUT_DIR/$$(QMAKESPEC)/bin/plugins

DEPENDPATH += $${MERKAARTOR_SRC_DIR}/interfaces
INCLUDEPATH += $${MERKAARTOR_SRC_DIR}/interfacesH

unix {
    # Prefix: base instalation directory
    isEmpty( PREFIX ) {
		PREFIX = /usr/local
	}

	target.path = $${PREFIX}/lib/Merkaartor/plugins
	INSTALLS += target
}
