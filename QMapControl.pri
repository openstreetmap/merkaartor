# Input
HEADERS += QMapControl/curve.h \
           QMapControl/geometry.h \
           QMapControl/IImageManager.h \
           QMapControl/imagemanager.h \
#           QMapControl/browserimagemanager.h \
           QMapControl/layer.h \
           QMapControl/layermanager.h \
           QMapControl/linestring.h \
           QMapControl/mapadapter.h \
           QMapControl/mapcontrol.h \
           QMapControl/mapnetwork.h \
           QMapControl/point.h \
           QMapControl/wmsmapadapter.h \
 QMapControl/circlepoint.h \
 QMapControl/imagepoint.h \
 QMapControl/gps_position.h \
 QMapControl/osmmapadapter.h \
 QMapControl/geometrylayer.h \
# QMapControl/yahoolegalmapadapter.h \
# QMapControl/yahoomapadapter.h \
# QMapControl/googlemapadapter.h \
# QMapControl/googlesatmapadapter.h \
           QMapControl/tilemapadapter.h 
SOURCES += QMapControl/curve.cpp \
           QMapControl/geometry.cpp \
           QMapControl/imagemanager.cpp \
#           QMapControl/browserimagemanager.cpp \
           QMapControl/layer.cpp \
           QMapControl/layermanager.cpp \
           QMapControl/linestring.cpp \
           QMapControl/mapadapter.cpp \
           QMapControl/mapcontrol.cpp \
           QMapControl/mapnetwork.cpp \
           QMapControl/point.cpp \
           QMapControl/wmsmapadapter.cpp \
 QMapControl/circlepoint.cpp \
 QMapControl/imagepoint.cpp \
 QMapControl/gps_position.cpp \
 QMapControl/osmmapadapter.cpp \
 QMapControl/geometrylayer.cpp \
# QMapControl/yahoolegalmapadapter.cpp \
# QMapControl/yahoomapadapter.cpp \
# QMapControl/googlemapadapter.cpp
# QMapControl/googlesatmapadapter.cpp
           QMapControl/tilemapadapter.cpp

QT += network  
