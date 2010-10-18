include(../../common.pri)
include(../common.pri)
include(qadastre.pri)

TARGET = $$qtLibraryTarget(MCadastreFranceBackgroundPlugin)
DEPENDPATH += $${MERKAARTOR_SRC_DIR}/interfaces
INCLUDEPATH += $${MERKAARTOR_SRC_DIR}/interfaces
HEADERS += \
    IMapAdapter.h \
    IImageManager.h \
    CadastreFrance.h

SOURCES += \
    CadastreFrance.cpp

