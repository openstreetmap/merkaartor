#Header files
HEADERS += \
./Command/Command.h \
./Command/DocumentCommands.h \
./Command/FeatureCommands.h \
./Command/RelationCommands.h \
./Command/RoadCommands.h \
./Command/TrackPointCommands.h \
./Interaction/CreateAreaInteraction.h \
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
./Map/ImportNGT.h \
./Map/ImportOSM.h \
./Map/ImportNGT.h \
./Map/MapDocument.h \
./Map/MapFeature.h \
./Map/Painting.h \
./Map/PreDefinedTags.h \
./Map/Projection.h \
./Map/Relation.h \
./Map/Road.h \
./Map/RoadManipulations.h \
./Map/TrackPoint.h \
./Map/TrackSegment.h \
./MapView.h \
./PaintStyle/EditPaintStyle.h \
./PaintStyle/PaintStyle.h \
./PaintStyle/PaintStyleEditor.h \ 
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
./Command/RelationCommands.cpp \
./Command/RoadCommands.cpp \
./Map/Coord.cpp \
./Map/DownloadOSM.cpp \
./Map/ExportOSM.cpp \
./Map/ImportGPX.cpp \
./Map/ImportOSM.cpp \
./Map/ImportNGT.cpp \
./Map/MapDocument.cpp \
./Map/MapFeature.cpp \
./Map/Painting.cpp \
./Map/PreDefinedTags.cpp \
./Map/Projection.cpp \
./Map/Relation.cpp \
./Map/Road.cpp \
./Map/RoadManipulations.cpp \
./Map/TrackPoint.cpp \
./Map/TrackSegment.cpp \
./MapView.cpp \
./Interaction/CreateAreaInteraction.cpp \
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
./PaintStyle/PaintStyleEditor.cpp \
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
./RelationProperties.ui \
./Interaction/CreateDoubleWayDock.ui \
./Interaction/CreateRoundaboutDock.ui \
./PaintStyle/PaintStyleEditor.ui


#Resource file(s)
RESOURCES += .\Icons\AllIcons.qrc



