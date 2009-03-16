# Input
HEADERS += \
           IImageManager.h \
           QMapControl/imagemanager.h \
#           QMapControl/browserimagemanager.h \
           QMapControl/layer.h \
           QMapControl/layermanager.h \
           QMapControl/mapadapter.h \
           QMapControl/mapnetwork.h \
           QMapControl/wmsmapadapter.h \
           QMapControl/tilemapadapter.h 

SOURCES += \
           QMapControl/IImageManager.cpp \
           QMapControl/imagemanager.cpp \
#           QMapControl/browserimagemanager.cpp \
           QMapControl/layer.cpp \
           QMapControl/layermanager.cpp \
           QMapControl/mapadapter.cpp \
           QMapControl/mapnetwork.cpp \
           QMapControl/wmsmapadapter.cpp \
           QMapControl/tilemapadapter.cpp

QT += network  
