DEFINES += OSMARENDER

#Header files
HEADERS += \
Render/OsmaRender.h 

#Source files
SOURCES += \
Render/OsmaRender.cpp 

QT += svg

win32 {
        DEFINES += LIBXML_STATIC LIBXSLT_STATIC
        LIBS += -Llibxml/win32/lib/ -llibxml2_a -llibxslt_a \
		-lzlib -liconv_a
        INCLUDEPATH += libxml/win32/include
}

unix {
        DEFINES += LIBXML_STATIC LIBXSLT_STATIC
        LIBS += -lxml2 -lxslt -lz
        INCLUDEPATH += /usr/include/libxml2
}

