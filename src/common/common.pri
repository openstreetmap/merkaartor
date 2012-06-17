INCLUDEPATH += $$MERKAARTOR_SRC_DIR/common
DEPENDPATH += $$MERKAARTOR_SRC_DIR/common

# Header files
HEADERS += Global.h \
    Coord.h \
    Document.h \
    MapTypedef.h \
    Painting.h \
    Projection.h \
    FeatureManipulations.h \
    MapView.h \
    TagModel.h \
    GotoDialog.h \
    TerraceDialog.h

# Source files
SOURCES += Global.cpp \
    Coord.cpp \
    Document.cpp \
    Painting.cpp \
    Projection.cpp \
    FeatureManipulations.cpp \
    MapView.cpp \
    TagModel.cpp \
    GotoDialog.cpp \
    TerraceDialog.cpp

# Forms
FORMS += AboutDialog.ui \
    DownloadMapDialog.ui \
    UploadMapDialog.ui \
    GotoDialog.ui \
    TerraceDialog.ui \
    MultiProperties.ui \
    PropertiesDialog.ui

# Resource file(s)
RESOURCES += $$MERKAARTOR_SRC_DIR/../Icons/AllIcons.qrc \
    $$MERKAARTOR_SRC_DIR/../share/share.qrc \
    $$MERKAARTOR_SRC_DIR/../Styles/Styles.qrc \
    $$MERKAARTOR_SRC_DIR/../Icons/TurnRestrictions/TurnRestrictions.qrc
