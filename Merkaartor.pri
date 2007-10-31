#Header files
HEADERS += \
./Command/Command.h \
./Command/DocumentCommands.h \
./Command/FeatureCommands.h \
./Command/RoadCommands.h \
./Command/TrackPointCommands.h \
./Interaction/CreateDoubleWayInteraction.h \
./Interaction/CreateNodeInteraction.h \
./Interaction/CreateRoundaboutInteraction.h \
./Interaction/CreateSingleWayInteraction.h \
./Interaction/EditInteraction.h \
./Interaction/Interaction.h \
./Interaction/MoveTrackPointInteraction.h \
./Interaction/ZoomInteraction.h \
./LayerDock.h \
./MainWindow.h \
./Map/Coord.h \
./Map/DownloadOSM.h \
./Map/ExportOSM.h \
./Map/ImportGPX.h \
./Map/ImportOSM.h \
./Map/MapDocument.h \
./Map/MapFeature.h \
./Map/Painting.h \
./Map/Projection.h \
./Map/Road.h \
./Map/TrackPoint.h \
./Map/TrackSegment.h \
./MapView.h \
./PaintStyle/EditPaintStyle.h \
./PaintStyle/PaintStyle.h \
./PropertiesDock.h \
./Sync/DirtyList.h \
./Sync/SyncOSM.h \
./TagModel.h \
./Utils/LineF.h \
./Utils/SlippyMapWidget.h

#Source files
SOURCES += \
./Command/Command.cpp \
./Command/DocumentCommands.cpp \
./Command/FeatureCommands.cpp \
./Command/TrackPointCommands.cpp \
./Command/RoadCommands.cpp \
./Map/Coord.cpp \
./Map/DownloadOSM.cpp \
./Map/ExportOSM.cpp \
./Map/ImportGPX.cpp \
./Map/ImportOSM.cpp \
./Map/MapDocument.cpp \
./Map/MapFeature.cpp \
./Map/Painting.cpp \
./Map/Projection.cpp \
./Map/Road.cpp \
./Map/TrackPoint.cpp \
./Map/TrackSegment.cpp \
./MapView.cpp \
./Interaction/CreateDoubleWayInteraction.cpp \
./Interaction/CreateNodeInteraction.cpp \
./Interaction/CreateSingleWayInteraction.cpp \
./Interaction/CreateRoundaboutInteraction.cpp \
./Interaction/EditInteraction.cpp \
./Interaction/Interaction.cpp \
./Interaction/MoveTrackPointInteraction.cpp \
./Interaction/ZoomInteraction.cpp \
./PaintStyle/EditPaintStyle.cpp \
./PaintStyle/PaintStyle.cpp \
./Sync/DirtyList.cpp \
./Sync/SyncOSM.cpp \
./Main.cpp \
./MainWindow.cpp \
./PropertiesDock.cpp \
./TagModel.cpp \
./LayerDock.cpp \
./Utils/SlippyMapWidget.cpp

#Forms
FORMS += \
./AboutDialog.ui \
./DownloadMapDialog.ui \
./MainWindow.ui \
./RoadProperties.ui \
./Sync/SyncListDialog.ui \
./TrackPointProperties.ui \
./UploadMapDialog.ui \
./SetPositionDialog.ui \
./MultiProperties.ui \
./Interaction/CreateDoubleWayDock.ui \
./Interaction/CreateRoundaboutDock.ui


#Resource file(s)
RESOURCES += .\Icons\AllIcons.qrc



