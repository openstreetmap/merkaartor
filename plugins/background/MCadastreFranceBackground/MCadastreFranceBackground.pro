include(../../common.pri)
include(../common.pri)
include(qadastre.pri)

TARGET = $$qtLibraryTarget(MCadastreFranceBackgroundPlugin)

DEPENDPATH += $${MERKAARTOR_SRC_DIR}/interfaces
INCLUDEPATH += $${MERKAARTOR_SRC_DIR}/interfaces
include ($${MERKAARTOR_SRC_DIR}/interfaces/Interfaces.pri)

HEADERS += \
    CadastreFrance.h

SOURCES += \
    CadastreFrance.cpp

