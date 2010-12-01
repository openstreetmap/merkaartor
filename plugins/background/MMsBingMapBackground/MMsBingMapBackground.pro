include(../../common.pri)
include(../common.pri)

TARGET = $$qtLibraryTarget(MMsBingMapBackgroundPlugin)
DEPENDPATH += $${MERKAARTOR_SRC_DIR}/interfaces
INCLUDEPATH += $${MERKAARTOR_SRC_DIR}/interfaces
include ($${MERKAARTOR_SRC_DIR}/interfaces/Interfaces.pri)

HEADERS += \
    mapadapter.h \
    msbingmapadapter.h

SOURCES += \
    mapadapter.cpp \
    msbingmapadapter.cpp

RESOURCES += \
    Resources.qrc
