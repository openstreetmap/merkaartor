include(../../common.pri)
include(../common.pri)
TARGET = $$qtLibraryTarget(MNavitBackgroundPlugin)
QT += network \
    xml \
    svg
INCLUDEPATH += $$MERKAARTOR_SRC_DIR/src
DEPENDPATH += $$MERKAARTOR_SRC_DIR/src
INCLUDEPATH += $$MERKAARTOR_SRC_DIR/src/PaintStyle $$MERKAARTOR_SRC_DIR/src/Utils
DEPENDPATH += $$MERKAARTOR_SRC_DIR/src/PaintStyle $$MERKAARTOR_SRC_DIR/src/Utils
include(../../../src/PaintStyle/PaintStyle.pri)

include(MNavitBackground.pri)

COMMON_DIR = $${MERKAARTOR_SRC_DIR}/binaries
win32 {
    INCLUDEPATH += $$COMMON_DIR/include
    LIBS += -L$$COMMON_DIR/lib
}

symbian {
  # Load predefined include paths (e.g. QT_PLUGINS_BASE_DIR) to be used in the pro-files
  load(data_caging_paths)

  # EPOCALLOWDLLDATA have to set true because Qt macros has initialised global data
  TARGET.EPOCALLOWDLLDATA=1

  # Defines plugin files into Symbian .pkg package
  pluginDep.sources = MNavitBackgroundPlugin.dll
  pluginDep.path = $$QT_PLUGINS_BASE_DIR/background
  DEPLOYMENT += pluginDep
}
