cmake_minimum_required(VERSION 3.19.0)

set(VCS_FALLBACK_VERSION 0.20.0)
include(cmake/vcs.cmake)
message(STATUS "Configuring Merkaartor ${VCS_VERSION}, commit: ${VCS_COMMIT}, dirty: ${VCS_DIRTY}.")

# Set a default build type if none was specified
if(NOT CMAKE_BUILD_TYPE AND NOT CMAKE_CONFIGURATION_TYPES)
  set(MERK_DEFAULT_BUILD_TYPE "Debug")
  message(STATUS "Setting build type to '${MERK_DEFAULT_BUILD_TYPE}' as none was specified.")
  set(CMAKE_BUILD_TYPE "${MERK_DEFAULT_BUILD_TYPE}" CACHE STRING "Choose the type of build." FORCE)
  # Set the possible values of build type for cmake-gui
  set_property(CACHE CMAKE_BUILD_TYPE PROPERTY STRINGS "Debug" "Release" "MinSizeRel" "RelWithDebInfo")
endif()


project(merkaartor VERSION ${VCS_VERSION})
# TODO: Use configure_file() instead of definitions when qmake is deprecated.
# Using definitions causes unnecessary rebuilds, as with each commit the hash
# changes compile flags for each and every file. However, it is kept for qmake
# compatibility.
# Using configure_file() allows us to store the metadata in a .cpp file, thus
# only rebuilding that one and linking.
set(SHARE_DIR ${CMAKE_INSTALL_PREFIX}/share/merkaartor)
set(LIB_DIR   ${CMAKE_INSTALL_PREFIX}/lib/merkaartor)
configure_file("${PROJECT_SOURCE_DIR}/cmake/build-metadata.cpp.in" "${PROJECT_BINARY_DIR}/build-metadata.cpp" @ONLY)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

enable_testing()

# TODO: Add support for sanitizers

set(CMAKE_EXPORT_COMPILE_COMMANDS ON)

# Find includes in corresponding build directories
set(CMAKE_INCLUDE_CURRENT_DIR ON)

# Instruct CMake to run moc automatically when needed
set(CMAKE_AUTOMOC ON)

# Create code from a list of Qt designer ui files
set(CMAKE_AUTOUIC ON)

set(CMAKE_AUTORCC ON)

##############################################
# Find Qt dependencies
##############################################
set(MERKAARTOR_QT_LIBS Svg Network NetworkAuth Xml Core Gui Concurrent PrintSupport Widgets Test)
set(MERKAARTOR_QT_TOOLS LinguistTools)
find_package(QT NAMES Qt5 Qt6 REQUIRED COMPONENTS Core)
message(STATUS " * Qt version found: ${QT_VERSION}")
if (${QT_VERSION_MAJOR} EQUAL 6)
    list(APPEND MERKAARTOR_QT_LIBS Core5Compat)
endif()
if (APPLE)
    # macdeployqt does not properly bundle dbus
    # see https://bugreports.qt.io/browse/QTBUG-127075
    list(APPEND MERKAARTOR_QT_LIBS DBus)
endif()

find_package(Qt${QT_VERSION_MAJOR} COMPONENTS ${MERKAARTOR_QT_LIBS} ${MERKAARTOR_QT_TOOLS} CONFIG REQUIRED)
set(MERKAARTOR_LINK_LIBS ${MERKAARTOR_QT_LIBS})
list(TRANSFORM MERKAARTOR_LINK_LIBS PREPEND Qt${QT_VERSION_MAJOR}::)


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

# starting with Qt5.15 we have a lot of deprecation warnings,
# likely to make porting to Qt6 easier.
# as long as we require linking to older Qt versions those warnings
# are clutter. Remove this define when we start porting to Qt6.
add_definitions(-DQT_NO_DEPRECATED_WARNINGS)

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
${PROJECT_BINARY_DIR}/build-metadata.cpp
${merkaartor_SRCS_PLATFORM}
src/Backend/MemoryBackend.cpp
src/Backend/MemoryBackend.h
src/GPS/qgps.h
src/GPS/qgpsdevice.cpp
src/GPS/qgpssatellitetracker.h
src/GPS/SatelliteStrengthView.cpp
src/GPS/qgpssatellitetracker.cpp
src/GPS/SatelliteStrengthView.h
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
src/PaintStyle/MasPaintStyle.h
src/PaintStyle/Painter.h
src/PaintStyle/PaintStyleEditor.ui
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
src/Tools/ActionsDialog.cpp
src/Tools/ActionsDialog.h
src/qextserialport/qextserialport.cpp
src/qextserialport/qextserialport.h
src/qextserialport/qextserialbase.cpp
src/qextserialport/qextserialbase.h
src/ImportExport/ImportNMEA.cpp
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
src/ImportExport/ImportNGT.h
src/ImportExport/ImportCSVDialog.h
src/ImportExport/ExportOSM.h
src/ImportExport/ImportOSM.cpp
src/ImportExport/ImportExportCSV.h
src/ImportExport/IImportExport.h
src/ImportExport/ExportGPX.cpp
src/ImportExport/ImportGPX.cpp
src/ImportExport/ImportExportGdal.cpp
src/MainWindow.h
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
src/Utils/SlippyMapWidget.cpp
src/Utils/SlippyMapWidget.h
src/Utils/Utils.h
src/Utils/OsmLink.cpp
src/Utils/OsmServer.cpp
src/Utils/OsmOAuth2Flow.cpp
src/Utils/OAuth2OOBDialog.ui
src/Utils/OAuth2OOBDialog.cpp
src/Utils/ShortcutOverrideFilter.cpp
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
src/Utils/BoundaryIcon.cpp
src/Utils/BoundaryIcon.h
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
src/QMapControl/WmscMapAdapter.cpp
src/QMapControl/mapnetwork.cpp
src/QMapControl/wmsmapadapter.h
src/QMapControl/tilemapadapter.cpp
src/QMapControl/wmsmapadapter.cpp
src/QMapControl/imagemanager.cpp
src/QMapControl/WmscMapAdapter.h
src/QMapControl/tilemapadapter.h
src/QMapControl/imagemanager.h
src/QMapControl/mapadapter.h
src/QMapControl/mapadapter.cpp
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
src/Docks/DirtyDock.ui
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
./Styles/Styles.qrc

)

if (NOT APPLE)
install( FILES ${CMAKE_SOURCE_DIR}/src/org.merkaartor.merkaartor.desktop          DESTINATION share/applications     )
install( FILES ${CMAKE_SOURCE_DIR}/src/org.merkaartor.merkaartor.appdata.xml      DESTINATION share/metainfo     )
install( FILES ${CMAKE_SOURCE_DIR}/Icons/8x8/merkaartor.png      DESTINATION share/icons/hicolor/8x8/apps     )
install( FILES ${CMAKE_SOURCE_DIR}/Icons/16x16/merkaartor.png    DESTINATION share/icons/hicolor/16x16/apps   )
install( FILES ${CMAKE_SOURCE_DIR}/Icons/22x22/merkaartor.png    DESTINATION share/icons/hicolor/22x22/apps   )
install( FILES ${CMAKE_SOURCE_DIR}/Icons/24x24/merkaartor.png    DESTINATION share/icons/hicolor/24x24/apps   )
install( FILES ${CMAKE_SOURCE_DIR}/Icons/32x32/merkaartor.png    DESTINATION share/icons/hicolor/32x32/apps   )
install( FILES ${CMAKE_SOURCE_DIR}/Icons/36x36/merkaartor.png    DESTINATION share/icons/hicolor/36x36/apps   )
install( FILES ${CMAKE_SOURCE_DIR}/Icons/40x40/merkaartor.png    DESTINATION share/icons/hicolor/40x40/apps   )
install( FILES ${CMAKE_SOURCE_DIR}/Icons/42x42/merkaartor.png    DESTINATION share/icons/hicolor/42x42/apps   )
install( FILES ${CMAKE_SOURCE_DIR}/Icons/48x48/merkaartor.png    DESTINATION share/icons/hicolor/48x48/apps   )
install( FILES ${CMAKE_SOURCE_DIR}/Icons/64x64/merkaartor.png    DESTINATION share/icons/hicolor/64x64/apps   )
install( FILES ${CMAKE_SOURCE_DIR}/Icons/72x72/merkaartor.png    DESTINATION share/icons/hicolor/72x72/apps   )
install( FILES ${CMAKE_SOURCE_DIR}/Icons/80x80/merkaartor.png    DESTINATION share/icons/hicolor/80x80/apps   )
install( FILES ${CMAKE_SOURCE_DIR}/Icons/96x96/merkaartor.png    DESTINATION share/icons/hicolor/96x96/apps   )
install( FILES ${CMAKE_SOURCE_DIR}/Icons/128x128/merkaartor.png  DESTINATION share/icons/hicolor/128x128/apps )
install( FILES ${CMAKE_SOURCE_DIR}/Icons/192x192/merkaartor.png  DESTINATION share/icons/hicolor/192x192/apps )
install( FILES ${CMAKE_SOURCE_DIR}/Icons/256x256/merkaartor.png  DESTINATION share/icons/hicolor/256x256/apps )
install( FILES ${CMAKE_SOURCE_DIR}/Icons/512x512/merkaartor.png  DESTINATION share/icons/hicolor/512x512/apps )
endif()

set(MERKAARTOR_SHARE_FILES
  ${CMAKE_SOURCE_DIR}/share/BookmarksList.xml
  ${CMAKE_SOURCE_DIR}/share/Projections.xml
  ${CMAKE_SOURCE_DIR}/share/TmsServersList.xml
  ${CMAKE_SOURCE_DIR}/share/WmsServersList.xml
)

install( FILES ${MERKAARTOR_SHARE_FILES} DESTINATION share/merkaartor)
set_source_files_properties(${MEKAARTOR_SHARE_FILES} PROPERTIES MACOSX_PACKAGE_LOCATION "Resources")

set(TS_FILES
  ${CMAKE_SOURCE_DIR}/translations/merkaartor_cs.ts
  ${CMAKE_SOURCE_DIR}/translations/merkaartor_de.ts
  ${CMAKE_SOURCE_DIR}/translations/merkaartor_en.ts
  ${CMAKE_SOURCE_DIR}/translations/merkaartor_es.ts
  ${CMAKE_SOURCE_DIR}/translations/merkaartor_fi.ts
  ${CMAKE_SOURCE_DIR}/translations/merkaartor_fr.ts
  ${CMAKE_SOURCE_DIR}/translations/merkaartor_hr.ts
  ${CMAKE_SOURCE_DIR}/translations/merkaartor_hu.ts
  ${CMAKE_SOURCE_DIR}/translations/merkaartor_id_ID.ts
  ${CMAKE_SOURCE_DIR}/translations/merkaartor_it.ts
  ${CMAKE_SOURCE_DIR}/translations/merkaartor_ja.ts
  ${CMAKE_SOURCE_DIR}/translations/merkaartor_nl.ts
  ${CMAKE_SOURCE_DIR}/translations/merkaartor_pl.ts
  ${CMAKE_SOURCE_DIR}/translations/merkaartor_pt_BR.ts
  ${CMAKE_SOURCE_DIR}/translations/merkaartor_ru.ts
  ${CMAKE_SOURCE_DIR}/translations/merkaartor_sv.ts
  ${CMAKE_SOURCE_DIR}/translations/merkaartor_uk.ts
  ${CMAKE_SOURCE_DIR}/translations/merkaartor_zh_TW.ts
)
qt_add_translation(QM_FILES
    ${TS_FILES}
)
install( FILES ${QM_FILES} DESTINATION share/merkaartor/translations )

find_program(LUPDATE lupdate)
add_custom_target(update-translations COMMAND ${LUPDATE} ${CMAKE_SOURCE_DIR}/src ${CMAKE_SOURCE_DIR}/plugins -source-language en -target-language en -ts ${CMAKE_SOURCE_DIR}/translations/merkaartor_en.ts)


##############################################
# Find 3rd party libraries, depending on options provided
##############################################

# We need GDAL and proj for core functionality
set(PKGCONFIG_REQUIRED_LIBS gdal proj)

option(ZBAR        "Enable ZBar usage in MWalkingPapersBackground and Geoimage."      OFF)
option(GEOIMAGE    "Enable Geoimage Dock (requires exiv2 library)."                   ON )
option(GPSD        "Enable GPS Dock (requires gpsd library)."                         OFF)
option(LIBPROXY    "Enable libproxy usage."                                           OFF)
option(WEBENGINE   "Enable the use of QtWeb engine (not supported on all platforms)"  OFF)
option(PROTOBUF    "Enable support for .osm.pbf format."                              ON)
option(EXTRA_TESTS "Enable extra tests that cannot be run automatically on CI build." ON )

message(STATUS "Build options (use -DOPT=ON/OFF to enable/disable):")
message(STATUS " * ZBAR        ${ZBAR}")
message(STATUS " * GEOIMAGE    ${GEOIMAGE}")
message(STATUS " * GPSD        ${GPSD}")
message(STATUS " * LIBPROXY    ${LIBPROXY}")
message(STATUS " * WEBENGINE   ${WEBENGINE}")
message(STATUS " * PROTOBUF    ${PROTOBUF}")
message(STATUS " * EXTRA_TESTS ${EXTRA_TESTS}")
message(STATUS "")

if (ZBAR)
    list(APPEND PKGCONFIG_REQUIRED_LIBS zbar)
    add_definitions(-DUSE_ZBAR=1) # MWalkingPapersBackground, Geoimage; actual macro is USE_ZBAR
endif()

if (GEOIMAGE)
    list(APPEND PKGCONFIG_REQUIRED_LIBS exiv2)
    list(APPEND merkaartor_SRCS src/Docks/GeoImageDock.cpp src/Docks/GeoImageDock.h)
    add_definitions(-DGEOIMAGE=1)
endif()

if (GPSD)
    list(APPEND PKGCONFIG_REQUIRED_LIBS libgps)
    add_definitions(-DUSE_GPS=1)
endif()

if (LIBPROXY)
    list(APPEND PKGCONFIG_REQUIRED_LIBS libproxy-1.0)
    add_definitions(-DUSE_LIBPROXY=1)
endif()

if (WEBENGINE)
    add_definitions(-DUSEWEBENGINE=1) # Actual macro is USE_WEBKIT
    # add_definitions(-DTHREADED_BROWSERIMAGEMANAGER=1) # TODO: Check if that even does something/works
endif()

if (PROTOBUF)
    find_package(Protobuf REQUIRED)
    list(APPEND PKGCONFIG_REQUIRED_LIBS zlib protobuf)
    add_definitions(-DUSE_PROTOBUF)
    protobuf_generate_cpp(PROTOBUF_SRCS PROTOBUF_HDRS src/ImportExport/fileformat.proto src/ImportExport/osmformat.proto)
    set(PROTOBUF_FILES
        src/ImportExport/ImportExportPBF.cpp
        src/ImportExport/ImportExportPBF.h
        ${PROTOBUF_SRCS} ${PROTOBUF_HDRS}
    )
endif()

# TODO: Check and remove or implement defines from code: PORTABLE, FRISIUS, NVIDIA_HACK, MOBILE, LIBPROXY, SYSTEM_QTSA
find_package(PkgConfig REQUIRED)
pkg_check_modules(PKGCONFIG_DEPS REQUIRED ${PKGCONFIG_REQUIRED_LIBS})
message(STATUS "pkg-config dependencies require the following flags:")
message(STATUS " * LDFLAGS:          ${PKGCONFIG_DEPS_LDFLAGS}")
message(STATUS " * LDFLAGS_OTHER:    ${PKGCONFIG_DEPS_LDFLAGS_OTHER}")
message(STATUS " * CFLAGS:           ${PKGCONFIG_DEPS_CFLAGS}")
message(STATUS " * CFLAGS_OTHER:     ${PKGCONFIG_DEPS_CFLAGS_OTHER}")
message(STATUS " * LINK DIRECTORIES: ${PKGCONFIG_DEPS_LIBRARY_DIRS}")
message(STATUS "")


list(APPEND MERKAARTOR_LINK_LIBS ${PKGCONFIG_DEPS_LIBRARIES})
link_directories(${PKGCONFIG_DEPS_LIBRARY_DIRS})

##############################################
# Create the Merkaartor binary
##############################################
if (APPLE)
  set(MERKAARTOR_ICON_FILE "${CMAKE_SOURCE_DIR}/Icons/merkaartor.icns")
  set_source_files_properties(${MERKAARTOR_ICON_FILE} PROPERTIES MACOSX_PACKAGE_LOCATION "Resources")
endif()

if (WIN32)
  set(MERKAARTOR_ICON_FILE "${CMAKE_SOURCE_DIR}/Icons/merkaartor-win32.rc")
endif()

add_executable(${PROJECT_NAME} WIN32 MACOSX_BUNDLE ${merkaartor_SRCS} src/Main.cpp ${MERKAARTOR_ICON_FILE} ${QM_FILES} ${PROTOBUF_FILES})
target_link_libraries(merkaartor ${MERKAARTOR_LINK_LIBS} )
target_compile_options(merkaartor PUBLIC ${PKGCONFIG_DEPS_CFLAGS})
install( TARGETS merkaartor RUNTIME DESTINATION bin BUNDLE DESTINATION .)

if (APPLE)
  # Additionally, we need to find libssh and add that include, as exiv2 depends on it.
  # Caused by: https://gitlab.kitware.com/cmake/cmake/-/issues/19120 (not fixed and probably won't be anytime soon)
  #find_path(LIBSSH_INCLUDE_DIR NAMES libssh.h)
  set(LIBSSH_INCLUDE_DIR "/usr/local/include")
endif()

set(MERKAARTOR_INCLUDE_DIRECTORIES
  ${PKGCONFIG_DEPS_INCLUDE_DIRS}
  ${LIBSSH_INCLUDE_DIR}
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
target_include_directories( merkaartor PUBLIC ${MERKAARTOR_INCLUDE_DIRECTORIES})


##############################################
# Build plugins
##############################################
include(plugins/plugins.cmake)

##############################################
# Setup packaging
##############################################
set (CPACK_PACKAGE_NAME "merkaartor")
set (CPACK_PACKAGE_VENDOR "merkaartor.org")
set (CPACK_PACKAGE_DESCRIPTION_SUMMARY "Merkaartor, OpenStreetMap editor.")
set (CPACK_PACKAGE_VERSION_MAJOR "${PROJECT_VERSION_MAJOR}")
set (CPACK_PACKAGE_VERSION_MINOR "${PROJECT_VERSION_MINOR}")
set (CPACK_PACKAGE_VERSION_PATCH "${PROJECT_VERSION_PATCH}")
set (CPACK_PACKAGE_VERSION "${VCS_DESCRIBE}")
set (CPACK_PACKAGE_INSTALL_DIRECTORY "Merkaartor")
set (CPACK_PACKAGE_EXECUTABLES "merkaartor;Merkaartor")
set (CPACK_RESOURCE_FILE_LICENSE "${CMAKE_SOURCE_DIR}/LICENSE")

##############################################
# Prepare deployment script
##############################################
configure_file("${PROJECT_SOURCE_DIR}/cmake/upload-artifacts.sh.in" "${PROJECT_BINARY_DIR}/upload-artifacts.sh" @ONLY)
configure_file("${PROJECT_SOURCE_DIR}/cmake/AppImageBuilder.yml.in" "${PROJECT_BINARY_DIR}/AppImageBuilder.yml" @ONLY)

##############################################
# Mac OS X specific deploy scripts
##############################################
if (APPLE)
  find_program(MACDEPLOYQT macdeployqt6)
  message("Found macdeployqt: ${MACDEPLOYQT}")
  configure_file("${PROJECT_SOURCE_DIR}/cmake/deploy-osx.cmake.in" "${PROJECT_BINARY_DIR}/deploy-osx.cmake" @ONLY)
  set (CPACK_PRE_BUILD_SCRIPTS "${PROJECT_BINARY_DIR}/deploy-osx.cmake")
  set (CPACK_GENERATOR "DragNDrop")
  set (CPACK_DMG_BACKGROUND_IMAGE       "${PROJECT_SOURCE_DIR}/cmake/CMakeDMGBackground.tif")
  set (CPACK_DMG_DS_STORE               "${PROJECT_SOURCE_DIR}/cmake/DS_Store.bin")
  #set (CPACK_DMG_DS_STORE_SETUP_SCRIPT "${PROJECT_SOURCE_DIR}/cmake/CMakeDMGSetup.scpt")
  set (MACOSX_BUNDLE_BUNDLE_NAME "merkaartor")
  set (MACOSX_BUNDLE_BUNDLE_VERSION "${VCS_DESCRIBE}")
  set (MACOSX_BUNDLE_COPYRIGHT 2021)
  set (MACOSX_BUNDLE_GUI_IDENTIFIER "be.merkaartor")
  set (MACOSX_BUNDLE_ICON_FILE "merkaartor.icns")
  set (MACOSX_BUNDLE_INFO_STRING "Merkaartor, OpenStreetMap editor.")
  set (MACOSX_BUNDLE_LONG_VERSION_STRING "${VCS_DESCRIBE} (${VCS_COMMIT})")
  set (MACOSX_BUNDLE_SHORT_VERSION_STRING "${VCS_DESCRIBE} (short)")
endif()

##############################################
# Windows specific deploy scripts
##############################################
if (WIN32)
  set (CPACK_VERBATIM_VARIABLES ON)
  set (CPACK_NSIS_MUI_ICON "${CMAKE_SOURCE_DIR}/Icons/Merkaartor_48x48.ico")
  set (CPACK_NSIS_MUI_WELCOMEFINISHPAGE_BITMAP "${CMAKE_SOURCE_DIR}/Icons/Merkaartor_installer.bmp")
  string(REGEX REPLACE "/" "\\\\" CPACK_NSIS_MUI_ICON "${CPACK_NSIS_MUI_ICON}")
  string(REGEX REPLACE "/" "\\\\" CPACK_NSIS_MUI_WELCOMEFINISHPAGE_BITMAP "${CPACK_NSIS_MUI_WELCOMEFINISHPAGE_BITMAP}")
  set (CPACK_NSIS_ENABLE_UNINSTALL_BEFORE_INSTALL ON)
  set (CPACK_NSIS_URL_INFO_ABOUT "http://merkaartor.be")

  # Workaround for NSIS bug in CMake that creates and checks only 32bit keys even for 64bit installers.
  # https://gitlab.kitware.com/cmake/cmake/-/issues/21960
  # Will be fixed by PR:
  # https://gitlab.kitware.com/cmake/cmake/-/merge_requests/5952
  set(CMAKE_MODULE_PATH "${CMAKE_SOURCE_DIR}/cmake/ModulesOverride")
  set(CPACK_NSIS_REGISTRY_VIEW 64)
  set(CPACK_NSIS_ENABLE_UNINSTALL_DONT_CHECK_32 ON)
  # End of workaround

  if (${QT_VERSION_MAJOR} EQUAL 6)
    find_program(WINDEPLOYQT windeployqt-qt6)
  else()
    find_program(WINDEPLOYQT windeployqt)
  endif()
  find_program(CYGCHECK cygcheck)
  configure_file("${PROJECT_SOURCE_DIR}/cmake/deploy-windows.cmake.in" "${CMAKE_BINARY_DIR}/deploy-windows.cmake" @ONLY)
  set (CPACK_PRE_BUILD_SCRIPTS "${CMAKE_BINARY_DIR}/deploy-windows.cmake")

  set (CPACK_GENERATOR "ZIP;NSIS")
endif()

include(CPack)

##############################################
# Tests
##############################################

# The test shall be placed in file "tests/<test_name>.cpp" file, followed by required includes
function(MERK_ADD_TEST test_name)
  add_executable(${test_name} tests/${test_name}.cpp ${ARGN})
  target_link_libraries(${test_name} ${MERKAARTOR_LINK_LIBS} Qt::Test )
  add_test(${test_name} ${test_name})
  target_include_directories(${test_name} PUBLIC ${MERKAARTOR_INCLUDE_DIRECTORIES})
endfunction()

MERK_ADD_TEST(test-projection src/common/Projection.cpp src/common/Coord.cpp)
MERK_ADD_TEST(test-OsmLink src/Utils/OsmLink.cpp src/common/Coord.cpp)

# Additional tests that use the main executable
if (EXTRA_TESTS)
add_test(NAME "import-geojson" COMMAND ${PROJECT_NAME} --test-import ${CMAKE_SOURCE_DIR}/tests/data/sample.geojson)
add_test(NAME "import-shapefile-point" COMMAND ${PROJECT_NAME} --test-import ${CMAKE_SOURCE_DIR}/tests/data/sample-shapefile/POINT.shp)
add_test(NAME "import-shapefile-polygon" COMMAND ${PROJECT_NAME} --test-import ${CMAKE_SOURCE_DIR}/tests/data/sample-shapefile/POLYGON.shp)
add_test(NAME "import-shapefile-polyline" COMMAND ${PROJECT_NAME} --test-import ${CMAKE_SOURCE_DIR}/tests/data/sample-shapefile/POLYLINE.shp)

if (CMAKE_SYSTEM_NAME MATCHES "Linux")
# Use the offscreen platform for tests with GUI to avoid opening/closing the windows. However, the platform is only supported by Qt on Linux
set_tests_properties(
    import-geojson
    import-shapefile-point
    import-shapefile-polygon
    import-shapefile-polyline
    PROPERTIES ENVIRONMENT QT_QPA_PLATFORM=offscreen
    )
endif()
endif()
