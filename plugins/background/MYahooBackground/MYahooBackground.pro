include(../../common.pri)
include(../common.pri)

TARGET = $$qtLibraryTarget(MYahooBackgroundPlugin)
DEPENDPATH += $${MERKAARTOR_SRC_DIR}/interfaces
INCLUDEPATH += $${MERKAARTOR_SRC_DIR}/interfaces
HEADERS += \
	IMapAdapter.h \
	IImageManager.h \
	mapadapter.h \
	tilemapadapter.h \
	yahoolegalmapadapter.h 

SOURCES += \ 
	mapadapter.cpp \
	tilemapadapter.cpp \
	yahoolegalmapadapter.cpp 

RESOURCES += \
	MYahooBackground.qrc

