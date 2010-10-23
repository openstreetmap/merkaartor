include(../../common.pri)
include(../common.pri)

TARGET = $$qtLibraryTarget(MYahooTiledBackgroundPlugin)
DEPENDPATH += $${MERKAARTOR_SRC_DIR}/interfaces
INCLUDEPATH += $${MERKAARTOR_SRC_DIR}/interfaces
include ($${MERKAARTOR_SRC_DIR}/interfaces/Interfaces.pri)

HEADERS += \
    yahootiledmapadapter.h

SOURCES += \
    yahootiledmapadapter.cpp

RESOURCES += \
    MYahooTiledBackground.qrc

