# Header files
HEADERS += ./IProgressWindow.h \
    ./MainWindow.h \
    ./Maps/Coord.h \
    ./Document.h \
    ./Maps/MapTypedef.h \
    ./Maps/Painting.h \
    ./Maps/Projection.h \
    ./Maps/FeatureManipulations.h \
    ./MapView.h \
    ./TagModel.h \
    ./Utils/LineF.h \
    ./Utils/ShortcutOverrideFilter.h \
    ./Utils/SlippyMapWidget.h \
    ./Utils/EditCompleterDelegate.h \
    ./Utils/PictureViewerDialog.h \
    ./Utils/PixmapWidget.h \
    ./Utils/SelectionDialog.h \
    ./Utils/SvgCache.h \
    ./Utils/MDiscardableDialog.h \
    ./GotoDialog.h \
    ./TerraceDialog.h \
    ../include/builtin-ggl/ggl/extensions/gis/projections/impl/geocent.h \
    ../interfaces/IMapAdapter.h \
    Utils/OsmLink.h \
    Utils/Utils.h \
    Utils/TagSelector.h \
    Utils/TagSelectorWidget.h \
    Utils/ProjectionChooser.h

# Source files
SOURCES += ./Maps/Coord.cpp \
    ./Document.cpp \
    ./Maps/Painting.cpp \
    ./Maps/Projection.cpp \
    ./Maps/FeatureManipulations.cpp \
    ./MapView.cpp \
    ./Main.cpp \
    ./MainWindow.cpp \
    ./TagModel.cpp \
    ./Utils/ShortcutOverrideFilter.cpp \
    ./Utils/SlippyMapWidget.cpp \
    ./Utils/EditCompleterDelegate.cpp \
    ./Utils/PictureViewerDialog.cpp \
    ./Utils/PixmapWidget.cpp \
    ./Utils/SelectionDialog.cpp \
    ./Utils/SvgCache.cpp \
    ./Utils/MDiscardableDialog.cpp \
    ./GotoDialog.cpp \
    ./TerraceDialog.cpp \
    ../include/builtin-ggl/ggl/extensions/gis/projections/impl/geocent.c \
    Utils/OsmLink.cpp \
    Utils/Utils.cpp \
    Utils/TagSelector.cpp \
    Utils/TagSelectorWidget.cpp \
    Utils/ProjectionChooser.cpp

# Forms
FORMS += ./AboutDialog.ui \
    ./DownloadMapDialog.ui \
    ./MainWindow.ui \
    ./UploadMapDialog.ui \
    ./GotoDialog.ui \
    ./TerraceDialog.ui \
    ./MultiProperties.ui \
    ./Utils/PictureViewerDialog.ui \
    ./Utils/SelectionDialog.ui \
    Utils/TagSelectorWidget.ui \
    PropertiesDialog.ui \
    Utils/ProjectionChooser.ui

# Resource file(s)
RESOURCES += ../Icons/AllIcons.qrc \
    ./Utils/Utils.qrc \
    ../share/share.qrc
OTHER_FILES += ../CHANGELOG
