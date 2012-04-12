INCLUDEPATH += $$MERKAARTOR_SRC_DIR/Layers
DEPENDPATH += $$MERKAARTOR_SRC_DIR/Layers

HEADERS += Layer.h \
    ImageMapLayer.h \
    LayerIterator.h \
    LayerWidget.h \
    LayerPrivate.h \
    Layers/OsmRenderLayer.h
SOURCES += Layer.cpp \
    ImageMapLayer.cpp \
    LayerWidget.cpp \
    Layers/OsmRenderLayer.cpp
FORMS += LayerWidget.ui \
    FilterEditDialog.ui \
    LicenseDisplayDialog.ui
