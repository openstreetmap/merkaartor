DEFINES += OSMARENDER

#Header files
HEADERS += \
Render/OsmaRender.h 

#Source files
SOURCES += \
Render/OsmaRender.cpp 

QT += svg

win32-msvc* {
        DEFINES += LIBXML_STATIC LIBXSLT_STATIC
        LIBS += -Llibxml/win32/lib/ -llibxml2_a -llibxslt_a \
		-lzlib -liconv_a
        INCLUDEPATH += libxml/win32/include
	QMAKE_LFLAGS += /NODEFAULTLIB:MSVCRT
}

unix {
        DEFINES += LIBXML_STATIC LIBXSLT_STATIC
        LIBS += -lxml2 -lxslt -lz
        INCLUDEPATH += /usr/include/libxml2
}

win32-g++ {
	message("osmarender not supported under mingw/win32")
}


