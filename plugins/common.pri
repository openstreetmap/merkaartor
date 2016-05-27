# PREFIX              - base prefix for installation (default: /usr/local)
# LIBDIR              - base directory for plugins (default: $$PREFIX/lib)
# NODEBUG             - no debug target

#Qt Version
QT_VERSION = $$[QT_VERSION]
QT_VERSION = $$split(QT_VERSION, ".")
QT_VER_MAJ = $$member(QT_VERSION, 0)
QT_VER_MIN = $$member(QT_VERSION, 1)

MERKAARTOR_SRC_DIR = $$PWD/..

CONFIG += debug_and_release

#Static config
include ($$MERKAARTOR_SRC_DIR/src/Config.pri)

DEFINES += VERSION=\"\\\"$$VERSION\\\"\"
#DEFINES += REVISION=\"\\\"$$REVISION\\\"\"

# Drop the version for now. If it contains anything else than numbers split by
# dots, the qt stuff generates invalid *.rc files for plugins.
VERSION=""

win32-msvc* {
    DEFINES += _USE_MATH_DEFINES
}

TEMPLATE = lib
CONFIG += plugin

QT += core xml network widgets

OUTPUT_DIR = $$OUT_PWD/../binaries
UI_DIR += tmp
MOC_DIR += tmp
RCC_DIR += tmp
DESTDIR = $$OUTPUT_DIR/bin/plugins

DEPENDPATH += $${MERKAARTOR_SRC_DIR}/interfaces $${MERKAARTOR_SRC_DIR}/src/Utils
INCLUDEPATH += $${MERKAARTOR_SRC_DIR}/interfaces $${MERKAARTOR_SRC_DIR}/src/Utils
INCLUDEPATH += $$MERKAARTOR_SRC_DIR/include
LIBS += -L$$OUTPUT_DIR

VPATH=$$INCLUDEPATH

macx {
    # Prefix: base instalation directory
    PREFIX = /Applications
    LIBDIR = $${PREFIX}/lib${LIB_SUFFIX}
    target.path = $${PREFIX}/merkaartor.app/Contents/plugins
    INSTALLS += target
}

unix:!macx {
    # Prefix: base instalation directory
    isEmpty( PREFIX ) {
        PREFIX = /usr/local
    }
    isEmpty( LIBDIR ) {
        LIBDIR = $${PREFIX}/lib${LIB_SUFFIX}
    }

    target.path = $${LIBDIR}/merkaartor/plugins
    INSTALLS += target
}
