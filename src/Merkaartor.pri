# MOBILE=1 - enable MOBILE
# GEOIMAGE=1 - enable geotagged images (requires exiv2)
# GPSD=1 - use gpsd as location provider
# NVIDIA_HACK=1 - used to solve nvidia specific slowdown
# GDAL=1 - enable GDAL (for, e.g., shapefile import; requires libgdal)
# FORCE_CUSTOM_STYLE=1 - force custom style (recommended on Linux until the "expanding dock" is solved upstream)
# FORCE_CUSTOM_STYLE=1 - force custom style (recommended on Linux until the "expanding dock" is solved upstream)
# Header files
include (Docks/Docks.pri)
HEADERS += ./Command/Command.h \
    ./Command/DocumentCommands.h \
    ./Command/FeatureCommands.h \
    ./Command/RelationCommands.h \
    ./Command/RoadCommands.h \
    ./Command/TrackSegmentCommands.h \
    ./Command/TrackPointCommands.h \
    ./Interaction/CreateAreaInteraction.h \
    ./Interaction/CreateDoubleWayInteraction.h \
    ./Interaction/CreateNodeInteraction.h \
    ./Interaction/CreateRoundaboutInteraction.h \
    ./Interaction/CreatePolygonInteraction.h \
    ./Interaction/CreateSingleWayInteraction.h \
    ./Interaction/EditInteraction.h \
    ./Interaction/Interaction.h \
    ./Interaction/MoveTrackPointInteraction.h \
    ./Interaction/RotateInteraction.h \
    ./Interaction/ZoomInteraction.h \
    ./LayerWidget.h \
    ./IProgressWindow.h \
    ./MainWindow.h \
    ./Maps/Coord.h \
    ./Maps/DownloadOSM.h \
    ./Maps/ExportOSM.h \
    ./Maps/ImportGPX.h \
    ./Maps/ImportNGT.h \
    ./Maps/ImportOSM.h \
    ./Maps/ImportNGT.h \
    ./Maps/LayerIterator.h \
    ./Maps/MapDocument.h \
    ./Maps/MapLayer.h \
    ./Maps/ImageMapLayer.h \
    ./Maps/MapTypedef.h \
    ./Maps/MapFeature.h \
    ./Maps/Painting.h \
    ./Maps/Projection.h \
    ./Maps/Relation.h \
    ./Maps/Road.h \
    ./Maps/FeatureManipulations.h \
    ./Maps/TrackPoint.h \
    ./Maps/TrackSegment.h \
    ./MapView.h \
    ./PaintStyle/EditPaintStyle.h \
    ./PaintStyle/PaintStyle.h \
    ./PaintStyle/PaintStyleEditor.h \
    ./PaintStyle/TagSelector.h \
    ./Sync/DirtyList.h \
    ./Sync/SyncOSM.h \
    ./TagModel.h \
    ./Preferences/MerkaartorPreferences.h \
    ./Preferences/PreferencesDialog.h \
    ./Preferences/WMSPreferencesDialog.h \
    ./Preferences/TMSPreferencesDialog.h \
    ./Preferences/ProjectionsList.h \
    ./Preferences/WmsServersList.h \
    ./Preferences/TmsServersList.h \
    ./Preferences/BookmarksList.h \
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
    ../include/ggl/extensions/gis/projections/impl/geocent.h \
    ../interfaces/IMapAdapter.h \
    Utils/SortAccordingToRenderingPriority.h

# Source files
SOURCES += ./Command/Command.cpp \
    ./Command/DocumentCommands.cpp \
    ./Command/FeatureCommands.cpp \
    ./Command/TrackPointCommands.cpp \
    ./Command/RelationCommands.cpp \
    ./Command/RoadCommands.cpp \
    ./Command/TrackSegmentCommands.cpp \
    ./Maps/Coord.cpp \
    ./Maps/DownloadOSM.cpp \
    ./Maps/ExportOSM.cpp \
    ./Maps/ImportGPX.cpp \
    ./Maps/ImportOSM.cpp \
    ./Maps/ImportNGT.cpp \
    ./Maps/MapDocument.cpp \
    ./Maps/MapLayer.cpp \
    ./Maps/ImageMapLayer.cpp \
    ./Maps/MapFeature.cpp \
    ./Maps/Painting.cpp \
    ./Maps/Projection.cpp \
    ./Maps/Relation.cpp \
    ./Maps/Road.cpp \
    ./Maps/FeatureManipulations.cpp \
    ./Maps/TrackPoint.cpp \
    ./Maps/TrackSegment.cpp \
    ./MapView.cpp \
    ./Interaction/CreateAreaInteraction.cpp \
    ./Interaction/CreateDoubleWayInteraction.cpp \
    ./Interaction/CreateNodeInteraction.cpp \
    ./Interaction/CreateSingleWayInteraction.cpp \
    ./Interaction/CreateRoundaboutInteraction.cpp \
    ./Interaction/CreatePolygonInteraction.cpp \
    ./Interaction/EditInteraction.cpp \
    ./Interaction/Interaction.cpp \
    ./Interaction/MoveTrackPointInteraction.cpp \
    ./Interaction/RotateInteraction.cpp \
    ./Interaction/ZoomInteraction.cpp \
    ./PaintStyle/EditPaintStyle.cpp \
    ./PaintStyle/PaintStyle.cpp \
    ./PaintStyle/PaintStyleEditor.cpp \
    ./PaintStyle/TagSelector.cpp \
    ./Sync/DirtyList.cpp \
    ./Sync/SyncOSM.cpp \
    ./Main.cpp \
    ./MainWindow.cpp \
    ./TagModel.cpp \
    ./LayerWidget.cpp \
    ./Utils/ShortcutOverrideFilter.cpp \
    ./Utils/SlippyMapWidget.cpp \
    ./Utils/EditCompleterDelegate.cpp \
    ./Utils/PictureViewerDialog.cpp \
    ./Utils/PixmapWidget.cpp \
    ./Utils/SelectionDialog.cpp \
    ./Utils/SvgCache.cpp \
    ./Utils/MDiscardableDialog.cpp \
    ./Preferences/MerkaartorPreferences.cpp \
    ./Preferences/PreferencesDialog.cpp \
    ./Preferences/WMSPreferencesDialog.cpp \
    ./Preferences/TMSPreferencesDialog.cpp \
    ./Preferences/ProjectionsList.cpp \
    ./Preferences/WmsServersList.cpp \
    ./Preferences/TmsServersList.cpp \
    ./Preferences/BookmarksList.cpp \
    ./GotoDialog.cpp \
    ../include/ggl/extensions/gis/projections/impl/geocent.c

# Forms
FORMS += ./AboutDialog.ui \
    ./DownloadMapDialog.ui \
    ./MainWindow.ui \
    ./Sync/SyncListDialog.ui \
    ./UploadMapDialog.ui \
    ./GotoDialog.ui \
    ./MultiProperties.ui \
    ./Interaction/CreateDoubleWayDock.ui \
    ./Interaction/CreateRoundaboutDock.ui \
    ./PaintStyle/PaintStyleEditor.ui \
    ./Preferences/PreferencesDialog.ui \
    ./Preferences/WMSPreferencesDialog.ui \
    ./Preferences/TMSPreferencesDialog.ui \
    ./Utils/PictureViewerDialog.ui \
    ./Utils/SelectionDialog.ui \
    ./ExportDialog.ui

# Resource file(s)
RESOURCES += ../Icons/AllIcons.qrc \
    ./Utils/Utils.qrc \
    ../share/share.qrc
