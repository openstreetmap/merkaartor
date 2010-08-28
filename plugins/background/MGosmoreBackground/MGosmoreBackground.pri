HEADERS += \
    IMapAdapter.h \
    IImageManager.h \
    GosmoreAdapter.h \
    libgosm.h \
    GosmoreFeature.h

SOURCES += \
    Utils/TagSelector.cpp \
    Utils/SvgCache.cpp \
    GosmoreAdapter.cpp \
    libgosm.cpp \
    GosmoreFeature.cpp

DEFINES += NOGTK

RESOURCES += \
    MGosmoreBackground.qrc
