include(../../common.pri)
include(../common.pri)

TARGET = $$qtLibraryTarget(MYahooTiledBackgroundPlugin)
DEPENDPATH += $${MERKAARTOR_SRC_DIR}/interfaces
INCLUDEPATH += $${MERKAARTOR_SRC_DIR}/interfaces
HEADERS += \
	IMapAdapter.h \
	IImageManager.h \
	yahootiledmapadapter.h

SOURCES += \ 
	yahootiledmapadapter.cpp

RESOURCES += \
	MYahooTiledBackground.qrc

