cmake_minimum_required(VERSION 3.1.0)

project(merkaartor VERSION 0.18.4)

include(cmake/vcs.cmake)
# TODO: Use configure_file() instead of definitions when qmake is deprecated.
# Using definitions causes unnecessary rebuilds, as with each commit the hash
# changes compile flags for each and every file. However, it is kept for qmake
# compatibility.
# Using configure_file() allows us to store the metadata in a .cpp file, thus
# only rebuilding that one and linking.
add_definitions(-DVERSION=${PROJECT_VERSION} -DREVISION=${VCS_DESCRIBE})
add_definitions(-DPRODUCT=Merkaartor)

message(WARNING "

CMake build is experimental and may be missing some features. Please use qmake for production builds.

")

# TODO: Add support for sanitizers

add_definitions(-DZBAR=1) # MWalkingPapersBackground, Geoimage
# TODO: Check and remove or implement defines from code: PORTABLE, FRISIUS, NVIDIA_HACK, MOBILE, LIBPROXY, SYSTEM_QTSA
add_definitions(-DGEOIMAGE=1)
add_definitions(-DGPSDLIB=1)
add_definitions(-DUSEWEBENGINE=1)
# add_definitions(-DTHREADED_BROWSERIMAGEMANAGER=1) # TODO: Check if that even does something/works

set(CMAKE_EXPORT_COMPILE_COMMANDS ON)

# Find includes in corresponding build directories
set(CMAKE_INCLUDE_CURRENT_DIR ON)

# Instruct CMake to run moc automatically when needed
set(CMAKE_AUTOMOC ON)

# Create code from a list of Qt designer ui files
set(CMAKE_AUTOUIC ON)

set(CMAKE_AUTORCC ON)

set(CMAKE_AUTOUIC_SEARCH_PATHS
${CMAKE_CURRENT_SOURCE_DIR}/src
${CMAKE_CURRENT_SOURCE_DIR}/src/GPS
${CMAKE_CURRENT_SOURCE_DIR}/src/Preferences
${CMAKE_CURRENT_SOURCE_DIR}/src/Render
${CMAKE_CURRENT_SOURCE_DIR}/src/PaintStyle
${CMAKE_CURRENT_SOURCE_DIR}/src/ImportExport
${CMAKE_CURRENT_SOURCE_DIR}/src/common
${CMAKE_CURRENT_SOURCE_DIR}/src/Layers
${CMAKE_CURRENT_SOURCE_DIR}/src/Utils
${CMAKE_CURRENT_SOURCE_DIR}/src/QToolBarDialog
${CMAKE_CURRENT_SOURCE_DIR}/src/NameFinder
${CMAKE_CURRENT_SOURCE_DIR}/src/Interactions
${CMAKE_CURRENT_SOURCE_DIR}/src/Sync
${CMAKE_CURRENT_SOURCE_DIR}/src/Docks
)


if (UNIX)
	set(merkaartor_SRCS_PLATFORM
		src/qextserialport/posix_qextserialport.cpp
		3rdparty/qtsingleapplication-2.6_1-opensource/src/qtlockedfile_unix.cpp
	)
elseif (WIN32)
	set(merkaartor_SRCS_PLATFORM
		src/qextserialport/win_qextserialport.cpp
		3rdparty/qtsingleapplication-2.6_1-opensource/src/qtlockedfile_win.cpp
	)
endif()


# Populate a CMake variable with the sources
set(merkaartor_SRCS
${merkaartor_SRCS_PLATFORM}
src/Backend/MemoryBackend.cpp
src/Backend/MemoryBackend.h
src/GPS/qgps.h
#src/GPS/GpsFix.cpp
src/GPS/qgpsdevice.cpp
src/GPS/qgpssatellitetracker.h
src/GPS/SatelliteStrengthView.cpp
src/GPS/qgpssatellitetracker.cpp
src/GPS/SatelliteStrengthView.h
#src/GPS/GpsFix.h
src/GPS/qgpsdevice.h
src/GPS/qgps.cpp
src/GPS/qgpsmainwindowui.ui
src/MainWindow.ui
src/Features/Way.h
src/Features/TrackSegment.cpp
src/Features/Node.cpp
src/Features/Feature.cpp
src/Features/Relation.h
src/Features/Way.cpp
src/Features/TrackSegment.h
src/Features/Features.h
src/Features/Node.h
src/Features/Feature.h
src/Features/Relation.cpp
src/Preferences/TMSPreferencesDialog.cpp
src/Preferences/WMSPreferencesDialog.h
src/Preferences/FilterList.cpp
src/Preferences/MerkaartorPreferences.cpp
src/Preferences/MerkaartorPreferences.h
src/Preferences/BookmarksList.h
src/Preferences/FilterPreferencesDialog.ui
src/Preferences/PreferencesDialog.h
src/Preferences/WmsServersList.cpp
src/Preferences/FilterList.h
src/Preferences/WMSPreferencesDialog.ui
src/Preferences/WMSPreferencesDialog.cpp
src/Preferences/FilterPreferencesDialog.cpp
src/Preferences/ProjPreferencesDialog.ui
src/Preferences/TmsServersList.h
src/Preferences/TmsServersList.cpp
src/Preferences/OsmServerWidget.ui
src/Preferences/TMSPreferencesDialog.h
src/Preferences/BookmarksList.cpp
src/Preferences/FilterPreferencesDialog.h
src/Preferences/TMSPreferencesDialog.ui
src/Preferences/ProjectionsList.cpp
src/Preferences/ProjPreferencesDialog.cpp
src/Preferences/PreferencesDialog.ui
src/Preferences/ProjPreferencesDialog.h
src/Preferences/WmsServersList.h
src/Preferences/ProjectionsList.h
src/Preferences/PreferencesDialog.cpp
src/Render/MapRenderer.h
src/Render/NativeRenderDialog.ui
src/Render/NativeRenderDialog.h
src/Render/NativeRenderDialog.cpp
src/Render/MapRenderer.cpp
src/PaintStyle/Painter.cpp
src/PaintStyle/MapCSSPaintstyle.cpp
src/PaintStyle/MasPaintStyle.h
src/PaintStyle/Painter.h
src/PaintStyle/MapCSSPaintstyle.h
src/PaintStyle/PaintStyleEditor.ui
src/PaintStyle/PrimitivePainter.cpp
src/PaintStyle/PrimitivePainter.h
src/PaintStyle/PaintStyleEditor.h
src/PaintStyle/MasPaintStyle.cpp
src/PaintStyle/FeaturePainter.cpp
src/PaintStyle/FeaturePainter.h
src/PaintStyle/PaintStyleEditor.cpp
src/PaintStyle/IPaintStyle.h
src/MainWindow.cpp
src/Commands/TrackSegmentCommands.cpp
src/Commands/WayCommands.cpp
src/Commands/NodeCommands.cpp
src/Commands/TrackSegmentCommands.h
src/Commands/NodeCommands.h
src/Commands/FeatureCommands.cpp
src/Commands/FeatureCommands.h
src/Commands/RelationCommands.cpp
src/Commands/DocumentCommands.cpp
src/Commands/DocumentCommands.h
src/Commands/WayCommands.h
src/Commands/Command.cpp
src/Commands/Command.h
src/Commands/RelationCommands.h
#src/Tools/QFatFs/QFatFs.h
#src/Tools/QFatFs/QFat.cpp
#src/Tools/QFatFs/QFatFile.cpp
#src/Tools/QFatFs/QFatFs.cpp
#src/Tools/QFatFs/QFat.h
#src/Tools/QFatFs/QFatFile.h
src/Tools/ActionsDialog.cpp
#src/Tools/RegionMapWidget.cpp
#src/Tools/RegionMapWidget.h
src/Tools/ActionsDialog.h
src/qextserialport/qextserialport.cpp
src/qextserialport/qextserialport.h
src/qextserialport/qextserialbase.cpp
src/qextserialport/qextserialbase.h
src/ImportExport/ImportNMEA.cpp
#src/ImportExport/ImportExportPBF.h
src/ImportExport/ImportNMEA.h
src/ImportExport/ImportExportGdal.h
src/ImportExport/IImportExport.cpp
src/ImportExport/ImportExportOSC.h
src/ImportExport/ImportOSM.h
src/ImportExport/ExportOSM.cpp
src/ImportExport/ExportDialog.ui
src/ImportExport/ImportNGT.cpp
src/ImportExport/ImportExportCSV.cpp
src/ImportExport/ImportGPX.h
src/ImportExport/ImportExportKML.h
src/ImportExport/ImportExportKML.cpp
src/ImportExport/ImportExportOSC.cpp
src/ImportExport/ExportGPX.h
src/ImportExport/ImportCSVDialog.cpp
src/ImportExport/ImportCSVDialog.ui
#src/ImportExport/fileformat.pb.h
#src/ImportExport/osmformat.pb.cc
#src/ImportExport/osmformat.pb.h
#src/ImportExport/fileformat.pb.cc
src/ImportExport/ImportNGT.h
src/ImportExport/ImportCSVDialog.h
src/ImportExport/ExportOSM.h
src/ImportExport/ImportOSM.cpp
src/ImportExport/ImportExportCSV.h
#src/ImportExport/ImportExportPBF.cpp
src/ImportExport/IImportExport.h
src/ImportExport/ExportGPX.cpp
src/ImportExport/ImportGPX.cpp
src/ImportExport/ImportExportGdal.cpp
src/MainWindow.h
src/Main.cpp
src/common/UploadMapDialog.ui
src/common/GotoDialog.ui
src/common/PropertiesDialog.ui
src/common/GotoDialog.h
src/common/MapTypedef.h
src/common/RelationProperties.ui
src/common/MapView.h
src/common/GotoDialog.cpp
src/common/TerraceDialog.h
src/common/TerraceDialog.cpp
src/common/TagModel.cpp
src/common/Document.h
src/common/Coord.h
src/common/Painting.h
src/common/FeatureManipulations.h
src/common/MultiProperties.ui
src/common/TerraceDialog.ui
src/common/Document.cpp
src/common/DownloadMapDialog.ui
src/common/FeatureManipulations.cpp
src/common/AboutDialog.ui
src/common/MapView.cpp
src/common/TagModel.h
src/common/Coord.cpp
src/common/Global.h
src/common/Global.cpp
src/common/Painting.cpp
src/common/Projection.h
src/common/Projection.cpp
src/Layers/OsmRenderLayer.cpp
src/Layers/LayerWidget.cpp
src/Layers/ImageMapLayer.h
src/Layers/OsmRenderLayer.h
src/Layers/Layer.cpp
src/Layers/LicenseDisplayDialog.ui
src/Layers/ImageMapLayer.cpp
src/Layers/LayerIterator.h
src/Layers/Layer.h
src/Layers/LayerWidget.h
src/Layers/LayerWidget.ui
src/Layers/FilterEditDialog.ui
src/Layers/LayerPrivate.h
src/Utils/ShortcutOverrideFilter.h
src/Utils/SelectionDialog.cpp
src/Utils/ProjectionChooser.h
src/Utils/TagSelectorWidget.ui
src/Utils/CheckBoxList.cpp
src/Utils/SlippyMapWidget.cpp
src/Utils/TagSelectorWidget.h
src/Utils/CheckBoxList.h
src/Utils/SlippyMapWidget.h
src/Utils/Utils.h
src/Utils/OsmLink.cpp
src/Utils/ShortcutOverrideFilter.cpp
src/Utils/TagSelectorWidget.cpp
src/Utils/LineF.h
src/Utils/SelectionDialog.ui
src/Utils/TagSelector.h
src/Utils/Utils.cpp
src/Utils/PictureViewerDialog.h
src/Utils/EditCompleterDelegate.cpp
src/Utils/ProjectionChooser.cpp
src/Utils/SvgCache.cpp
src/Utils/MDiscardableDialog.cpp
src/Utils/ProjectionChooser.ui
src/Utils/PictureViewerDialog.ui
src/Utils/PixmapWidget.h
src/Utils/OsmLink.h
src/Utils/EditCompleterDelegate.h
src/Utils/PictureViewerDialog.cpp
src/Utils/MDiscardableDialog.h
src/Utils/SelectionDialog.h
src/Utils/PixmapWidget.cpp
src/Utils/TagSelector.cpp
src/Utils/SvgCache.h
src/Utils/RemoteControlServer.cpp
src/QToolBarDialog/qttoolbardialog.h
src/QToolBarDialog/qttoolbardialog.cpp
src/QToolBarDialog/qttoolbardialog.ui
src/NameFinder/namefinderwidget.ui
src/NameFinder/httpquery.h
src/NameFinder/namefindertablemodel.cpp
src/NameFinder/xmlstreamreader.cpp
src/NameFinder/NameFinderResult.h
src/NameFinder/namefindertablemodel.h
src/NameFinder/httpquery.cpp
src/NameFinder/namefinderwidget.h
src/NameFinder/xmlstreamreader.h
src/NameFinder/namefinderwidget.cpp
src/TagTemplate/TagTemplate.cpp
src/TagTemplate/TagTemplate.h
src/Interactions/ScaleInteraction.h
src/Interactions/CreateAreaInteraction.cpp
src/Interactions/Interaction.h
src/Interactions/CreateAreaInteraction.h
src/Interactions/EditInteraction.h
src/Interactions/ScaleInteraction.cpp
src/Interactions/CreateDoubleWayInteraction.cpp
src/Interactions/EditInteraction.cpp
src/Interactions/CreateDoubleWayInteraction.h
src/Interactions/RotateInteraction.h
src/Interactions/ZoomInteraction.h
src/Interactions/CreateSingleWayInteraction.cpp
src/Interactions/Interaction.cpp
src/Interactions/ExtrudeInteraction.cpp
src/Interactions/ZoomInteraction.cpp
src/Interactions/RotateInteraction.cpp
src/Interactions/CreateDoubleWayDock.ui
src/Interactions/CreatePolygonInteraction.cpp
src/Interactions/MoveNodeInteraction.h
src/Interactions/MoveNodeInteraction.cpp
src/Interactions/CreatePolygonInteraction.h
src/Interactions/CreateNodeInteraction.cpp
src/Interactions/CreateRoundaboutDock.ui
src/Interactions/CreateRoundaboutInteraction.h
src/Interactions/ExtrudeInteraction.h
src/Interactions/BuildBridgeInteraction.h
src/Interactions/CreateRoundaboutInteraction.cpp
src/Interactions/CreateNodeInteraction.h
src/Interactions/CreateSingleWayInteraction.h
src/Interactions/BuildBridgeInteraction.cpp
src/Sync/SyncListDialog.ui
src/Sync/DirtyList.cpp
src/Sync/DownloadOSM.h
src/Sync/DirtyListExecutorOSC.cpp
src/Sync/DirtyListExecutorOSC.h
src/Sync/DownloadOSM.cpp
src/Sync/DirtyList.h
#src/QMapControl/mapcontrol.cpp
src/QMapControl/WmscMapAdapter.cpp
src/QMapControl/mapnetwork.cpp
src/QMapControl/wmsmapadapter.h
src/QMapControl/tilemapadapter.cpp
src/QMapControl/wmsmapadapter.cpp
#src/QMapControl/imagepoint.h
src/QMapControl/imagemanager.cpp
src/QMapControl/WmscMapAdapter.h
src/QMapControl/tilemapadapter.h
src/QMapControl/imagemanager.h
#src/QMapControl/mapcontrol.h
src/QMapControl/mapadapter.h
src/QMapControl/mapadapter.cpp
#src/QMapControl/imagepoint.cpp
src/QMapControl/mapnetwork.h
src/Docks/PropertiesDock.cpp
src/Docks/FeaturesDock.h
src/Docks/StyleDock.ui
src/Docks/MinimumRoadProperties.ui
src/Docks/InfoDock.h
src/Docks/FeaturesDock.ui
src/Docks/MinimumRelationProperties.ui
src/Docks/PropertiesDock.h
src/Docks/DirtyDock.h
src/Docks/StyleDock.cpp
src/Docks/GeoImageDock.cpp
src/Docks/DirtyDock.ui
src/Docks/GeoImageDock.h
src/Docks/MDockAncestor.cpp
src/Docks/MinimumTrackPointProperties.ui
src/Docks/StyleDock.h
src/Docks/PhotoLoadErrorDialog.ui
src/Docks/DirtyDock.cpp
src/Docks/InfoDock.cpp
src/Docks/FeaturesDock.cpp
src/Docks/LayerDock.cpp
src/Docks/MDockAncestor.h
src/Docks/LayerDock.h
interfaces/IImageManager.cpp
interfaces/IImageManager.h
interfaces/IMerkMainWindow.h
interfaces/IProjection.h
interfaces/IProgressWindow.h
interfaces/ILayer.h
interfaces/IMapAdapterFactory.h
interfaces/IFeature.h
interfaces/IMapWatermark.h
interfaces/IImageManager.cpp
interfaces/IRenderer.h
interfaces/IDocument.h
interfaces/Interfaces.pri
interfaces/IBackend.h
interfaces/IMapAdapter.h
3rdparty/qtsingleapplication-2.6_1-opensource/src/qtlockedfile.cpp
3rdparty/qtsingleapplication-2.6_1-opensource/src/qtsingleapplication.cpp
3rdparty/qtsingleapplication-2.6_1-opensource/src/qtlocalpeer.cpp
3rdparty/qtsingleapplication-2.6_1-opensource/src/qtsinglecoreapplication.cpp
3rdparty/qtsingleapplication-2.6_1-opensource/src/qtlocalpeer.h
3rdparty/qtsingleapplication-2.6_1-opensource/src/qtsingleapplication.h
3rdparty/qtsingleapplication-2.6_1-opensource/src/qtlockedfile.h
3rdparty/qtsingleapplication-2.6_1-opensource/src/qtsinglecoreapplication.h
./src/Utils/Utils.qrc
./Icons/AllIcons.qrc
./Icons/QToolBarDialog/qttoolbardialog.qrc
./Templates/Templates.qrc
./share/share.qrc
./Styles/Styles.qrc

)

# Find the QtWidgets library
find_package(Qt5 COMPONENTS Svg Network Xml Core Gui Concurrent PrintSupport Widgets CONFIG REQUIRED)
find_package(PkgConfig REQUIRED)
pkg_check_modules(EXIV2 REQUIRED exiv2 gdal proj)




# Tell CMake to create the helloworld executable
add_executable(merkaartor ${merkaartor_SRCS})
# Use the Widgets module from Qt 5
target_link_libraries(merkaartor Qt5::Svg Qt5::Network Qt5::Xml Qt5::Core Qt5::Gui Qt5::Concurrent Qt5::PrintSupport Qt5::Widgets ${EXIV2_LIBRARIES} )
target_compile_options(merkaartor PUBLIC ${EXIV2_CFLAGS_OTHER})
install( TARGETS merkaartor RUNTIME DESTINATION bin )

target_include_directories( merkaartor PUBLIC
${EXIV2_INCLUDE_DIRS}
${CMAKE_CURRENT_SOURCE_DIR}/interfaces
${CMAKE_CURRENT_SOURCE_DIR}/include
${CMAKE_CURRENT_SOURCE_DIR}/src
${CMAKE_CURRENT_SOURCE_DIR}/src/GPS
${CMAKE_CURRENT_SOURCE_DIR}/src/Backend
${CMAKE_CURRENT_SOURCE_DIR}/src/Preferences
${CMAKE_CURRENT_SOURCE_DIR}/src/Render
${CMAKE_CURRENT_SOURCE_DIR}/src/PaintStyle
${CMAKE_CURRENT_SOURCE_DIR}/src/ImportExport
${CMAKE_CURRENT_SOURCE_DIR}/src/common
${CMAKE_CURRENT_SOURCE_DIR}/src/Layers
${CMAKE_CURRENT_SOURCE_DIR}/src/Utils
${CMAKE_CURRENT_SOURCE_DIR}/src/QToolBarDialog
${CMAKE_CURRENT_SOURCE_DIR}/src/NameFinder
${CMAKE_CURRENT_SOURCE_DIR}/src/Interactions
${CMAKE_CURRENT_SOURCE_DIR}/src/Sync
${CMAKE_CURRENT_SOURCE_DIR}/src/Docks
${CMAKE_CURRENT_SOURCE_DIR}/src/Features
${CMAKE_CURRENT_SOURCE_DIR}/src/Commands
${CMAKE_CURRENT_SOURCE_DIR}/src/qextserialport
${CMAKE_CURRENT_SOURCE_DIR}/src/QMapControl
${CMAKE_CURRENT_SOURCE_DIR}/src/TagTemplate
${CMAKE_CURRENT_SOURCE_DIR}/3rdparty/qtsingleapplication-2.6_1-opensource/src
    )

include(plugins/plugins.cmake)
