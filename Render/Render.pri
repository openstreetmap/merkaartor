#Header files
HEADERS += \
	NativeRenderDialog.h 

#Source files
SOURCES += \
	NativeRenderDialog.cpp 

#Forms
FORMS += \
	NativeRenderDialog.ui

count(OSMARENDER,1) {

	DEFINES += OSMARENDER

	#Header files
	HEADERS += \
	OsmaRenderDialog.h 

	#Source files
	SOURCES += \
	OsmaRenderDialog.cpp 

	#Resource file(s)
	RESOURCES += osmarender.qrc 

	#Forms
	FORMS += \
	OsmaRenderDialog.ui

	QT += svg

	win32 {
		INCLUDEPATH += $$OUTPUT_DIR/include/libxml $$OUTPUT_DIR/include/libxslt
	}

	win32-msvc* {
		debug {
			DEFINES += LIBXML_STATIC LIBXSLT_STATIC
			LIBS += -llibxml2_a -llibxslt_a 
			QMAKE_LFLAGS += /NODEFAULTLIB:MSVCRT
		}
		release {
			LIBS += -llibxml2 -llibxslt 
		}
	}

	win32-g++ {
			LIBS += -llibxml2 -llibxslt 
	}

	unix {
			DEFINES += LIBXML_STATIC LIBXSLT_STATIC
			LIBS += -lxml2 -lxslt -lz
			INCLUDEPATH += /usr/include/libxml2
	}
}


