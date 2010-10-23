include(../../common.pri)
include(../common.pri)

TARGET = $$qtLibraryTarget(MYahooBackgroundPlugin)
DEPENDPATH += $${MERKAARTOR_SRC_DIR}/interfaces
INCLUDEPATH += $${MERKAARTOR_SRC_DIR}/interfaces
include ($${MERKAARTOR_SRC_DIR}/interfaces/Interfaces.pri)

HEADERS += \
    yahoolegalmapadapter.h

SOURCES += \
    yahoolegalmapadapter.cpp

RESOURCES += \
    MYahooBackground.qrc

