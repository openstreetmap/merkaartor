INCLUDEPATH += Layers
DEPENDPATH += Layers
HEADERS += Layer.h \
    ImageMapLayer.h \
    LayerIterator.h \
    LayerWidget.h \
    Layers/LayerPrivate.h
SOURCES += Layer.cpp \
    ImageMapLayer.cpp \
    LayerWidget.cpp
FORMS += LayerWidget.ui \
    FilterEditDialog.ui \
    Layers/LicenseDisplayDialog.ui
