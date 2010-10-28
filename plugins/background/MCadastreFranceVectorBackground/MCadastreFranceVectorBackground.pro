include(../../common.pri)
include(../common.pri)
include(VectorCadastre.pri)

QT += webkit
TARGET = $$qtLibraryTarget(MCadastreFranceVectorBackgroundPlugin)

DEPENDPATH += $${MERKAARTOR_SRC_DIR}/interfaces
INCLUDEPATH += $${MERKAARTOR_SRC_DIR}/interfaces
include ($${MERKAARTOR_SRC_DIR}/interfaces/Interfaces.pri)

HEADERS += \
    CadastreFranceVector.h

SOURCES += \
    CadastreFranceVector.cpp

