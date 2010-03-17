# Input
HEADERS += \
		   IImageManager.h \
		   QMapControl/imagemanager.h \
		   QMapControl/mapadapter.h \
		   QMapControl/mapnetwork.h \
		   QMapControl/wmsmapadapter.h \
		   QMapControl/WmscMapadapter.h \
		   QMapControl/tilemapadapter.h

SOURCES += \
		   QMapControl/IImageManager.cpp \
		   QMapControl/imagemanager.cpp \
		   QMapControl/mapadapter.cpp \
		   QMapControl/mapnetwork.cpp \
		   QMapControl/wmsmapadapter.cpp \
		   QMapControl/WmscMapadapter.cpp \
		   QMapControl/tilemapadapter.cpp

QT += network

!contains(NOUSEWEBKIT,1) {
	greaterThan(QT_VER_MAJ, 3) : greaterThan(QT_VER_MIN, 3) {
		DEFINES += USE_WEBKIT
		SOURCES += QMapControl/browserimagemanager.cpp
		HEADERS += QMapControl/browserimagemanager.h
		QT += webkit
		contains(THREADED_BROWSERIMAGEMANAGER,1): DEFINES += BROWSERIMAGEMANAGER_IS_THREADED
	}
}

