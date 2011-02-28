VERSION = 1.0.0
QT += network \
    xml \
    svg

DEFINES += _MOBILE
MOBILE = 1

symbian: {
    CONFIG(release): DEFINES += QT_NO_DEBUG_OUTPUT
}

# DEFINES += LOG_TO_FILE
MERKAARTOR_SRC_DIR = $$PWD/../src
COMMON_DIR = $$MERKAARTOR_SRC_DIR/binaries
OUTPUT_DIR = $$MERKAARTOR_SRC_DIR/binaries
DESTDIR = $$OUTPUT_DIR/bin

INCLUDEPATH += $$PWD/src $$PWD/../include $$PWD/../interfaces $MOC_DIR
DEPENDPATH += $$PWD/../interfaces

win32 {
  INCLUDEPATH += $$PWD/../binaries/include
  LIBS += -L$$PWD/../binaries/lib
}

#win32-msvc* {
#    LIBS += -lzlib
#} else {
#    LIBS += -lz
#}

#Qt Version
QT_VERSION = $$[QT_VERSION]
QT_VERSION = $$split(QT_VERSION, ".")
QT_VER_MAJ = $$member(QT_VERSION, 0)
QT_VER_MIN = $$member(QT_VERSION, 1)

DEFINES += VERSION=0.1
DEFINES += REVISION=

#Include file(s)
include(../src/common/common.pri)
include(../src/Backend/Backend.pri)
include(../interfaces/Interfaces.pri)
include(../src/PaintStyle/PaintStyle.pri)
include(../src/PaintStyle/PaintStyleEditor.pri)
include(../src/Features/Features.pri)
include(../src/Layers/Layers.pri)
include(../src/Preferences/Preferences.pri)
include(../src/Sync/Sync.pri)
include(../src/Commands/Commands.pri)
include(../src/Interactions/Interactions.pri)
include (../src/Docks/Docks.pri)
include(../src/QMapControl.pri)
include(../src/ImportExport/ImportExport.pri)
include(../src/Render/Render.pri)
isEmpty(MOBILE) {
  include(../src/qextserialport/qextserialport.pri)
}
include(../src/GPS/GPS.pri)
include(../src/Tools/Tools.pri)
include(../src/TagTemplate/TagTemplate.pri)
include(../src/NameFinder/NameFinder.pri)
include(../src/Utils/Utils.pri)

HEADERS += \
    src/TitleBar.h \
    src/HomeView.h \
    src/AddressBar.h \
    src/BookmarksView.h \
    src/BookmarksDelegate.h \
    src/ZoomStrip.h \
    src/ControlStrip.h \
    src/BookmarkBar.h \
    src/ControlButton.h \
    MyPreferences.h \
    MouseMachine/MouseMachine.h \
    MyMessageHandler.h \
    src/BookmarksModel.h \
    src/MainWindow.h \
    src/BookmarkItem.h \
    src/ViewMenu.h

SOURCES += src/Main.cpp \
    src/TitleBar.cpp \
    src/HomeView.cpp \
    src/AddressBar.cpp \
    src/BookmarksView.cpp \
    src/BookmarksDelegate.cpp \
    src/ZoomStrip.cpp \
    src/ControlStrip.cpp \
    src/BookmarkBar.cpp \
    MyPreferences.cpp \
    MouseMachine/MouseMachine.cpp \
    src/BookmarksModel.cpp \
    src/MainWindow.cpp \
    src/BookmarkItem.cpp \
    src/ViewMenu.cpp

FORMS += MyPreferences.ui \
    src/BookmarkBar.ui \
    src/ViewMenu.ui
RESOURCES += \
    # src/semperWeb.qrc \
    src/retinaIcons.qrc \
    src/resources/resources.qrc

symbian {
    TARGET.UID3 = 0x200398E6

    ICON = src/resources/Merkaartor.svg

    RSS_RULES = "group_name=\"SemperPax\";"
    vendorinfo = "; Localised Vendor name" \
        "%{\"SemperPax\"}" \
        " " \
        "; Unique Vendor name" \
        ":\"SemperPax\"" \
        " "
    default_deployment.pkg_prerules += vendorinfo
    LIBS += -lesock \
        -linsock
    TARGET.CAPABILITY = NetworkServices SwEvent ReadDeviceData Location
    TARGET.EPOCHEAPSIZE = 0x500000 \
        0x5000000
}
