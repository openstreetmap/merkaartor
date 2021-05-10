
if (APPLE)
set(PLUGINS_INSTALL_POSTFIX "merkaartor.app/Contents/plugins")
else()
set(PLUGINS_INSTALL_POSTFIX "lib/merkaartor/plugins")
endif()

function(MerkaartorAddPlugin)
    # Parse the arguments using CMake built-in function
    cmake_parse_arguments(PARSE_ARGV 0 PLUGIN "" "NAME;DESTINATION" "SOURCES")

    # Note: Adding IMapAdapter hints AUTOMOC to run the moc on there and include the qt metaclass in the library.
    # This appears to be only necessary on windows, due to linker differences.
    add_library(${PLUGIN_NAME} SHARED ${PLUGIN_SOURCES} ${PROJECT_SOURCE_DIR}/interfaces/IMapAdapter.h)
    target_compile_options(${PLUGIN_NAME} PRIVATE ${COMPILE_OPTIONS})
    target_compile_definitions(${PLUGIN_NAME} PRIVATE NO_PREFS)
    target_include_directories(${PLUGIN_NAME} PRIVATE 
        ${PKGCONFIG_DEPS_INCLUDE_DIRS}
        ${PROJECT_SOURCE_DIR}/interfaces
        # TODO: The rest of includes are necessary for ProjectionChooser.cpp
        # used in MGdalBackground and MGeoTiffBackground. This probably needs
        # to be fixed, probably by giving ProjectionChooser an interface?
        ${PROJECT_SOURCE_DIR}/src/Utils
        ${PROJECT_SOURCE_DIR}/src/Preferences
        ${PROJECT_SOURCE_DIR}/src/common
    )
    target_link_libraries(${PLUGIN_NAME} Qt5::Svg Qt5::Network Qt5::Xml Qt5::Core Qt5::Gui Qt5::Concurrent Qt5::PrintSupport Qt5::Widgets ${PKGCONFIG_DEPS_LIBRARIES} )
    install(TARGETS ${PLUGIN_NAME} LIBRARY DESTINATION ${PLUGINS_INSTALL_POSTFIX}/${PLUGIN_DESTINATION})
endfunction()

add_definitions("-DPLUGINS_DIR=${CMAKE_INSTALL_PREFIX}/${PLUGINS_INSTALL_POSTFIX}")

MerkaartorAddPlugin(NAME MGdalBackgroundPlugin DESTINATION background SOURCES
    ${PROJECT_SOURCE_DIR}/plugins/background/MGdalBackground/GdalAdapter.cpp
    ${PROJECT_SOURCE_DIR}/src/Utils/ProjectionChooser.cpp
    #${PROJECT_SOURCE_DIR}/plugins/background/MGdalBackground/GdalAdapter.json
)

MerkaartorAddPlugin(NAME MGeoTiffBackgroundPlugin DESTINATION background SOURCES
    ${PROJECT_SOURCE_DIR}/plugins/background/MGeoTiffBackground/GeoTiffAdapter.cpp
    ${PROJECT_SOURCE_DIR}/src/Utils/ProjectionChooser.cpp
    #${PROJECT_SOURCE_DIR}/plugins/background/MGeoTiffBackground/GeoTiffAdapter.json
)

MerkaartorAddPlugin(NAME MMsBingMapBackgroundPlugin DESTINATION background SOURCES
    ${PROJECT_SOURCE_DIR}/plugins/background/MMsBingMapBackground/Resources.qrc
    ${PROJECT_SOURCE_DIR}/plugins/background/MMsBingMapBackground/msbingmapadapter.cpp
    ${PROJECT_SOURCE_DIR}/plugins/background/MMsBingMapBackground/mapadapter.cpp
)

# Plugin not maintained for a long time.
# MerkaartorAddPlugin(NAME MCadastreFranceBackgroundPlugin DESTINATION background SOURCES
#     ${PROJECT_SOURCE_DIR}/plugins/background/MCadastreFranceBackground/qadastre
#     ${PROJECT_SOURCE_DIR}/plugins/background/MCadastreFranceBackground/qadastre/cadastrewrapper.cpp
#     ${PROJECT_SOURCE_DIR}/plugins/background/MCadastreFranceBackground/qadastre/COPYING.txt
#     ${PROJECT_SOURCE_DIR}/plugins/background/MCadastreFranceBackground/qadastre/searchdialog.ui
#     ${PROJECT_SOURCE_DIR}/plugins/background/MCadastreFranceBackground/qadastre/searchdialog.cpp
#     ${PROJECT_SOURCE_DIR}/plugins/background/MCadastreFranceBackground/qadastre/tile.cpp
#     ${PROJECT_SOURCE_DIR}/plugins/background/MCadastreFranceBackground/qadastre/cadastrebrowser.ui
#     ${PROJECT_SOURCE_DIR}/plugins/background/MCadastreFranceBackground/qadastre/city.cpp
#     ${PROJECT_SOURCE_DIR}/plugins/background/MCadastreFranceBackground/qadastre/cadastrebrowser.cpp
#     ${PROJECT_SOURCE_DIR}/plugins/background/MCadastreFranceBackground/qadastre/main.cpp
#     ${PROJECT_SOURCE_DIR}/plugins/background/MCadastreFranceBackground/CadastreFrance.cpp
# )

MerkaartorAddPlugin(NAME MWalkingPapersBackgroundPlugin DESTINATION background SOURCES
    #${PROJECT_SOURCE_DIR}/plugins/background/MWalkingPapersBackground/WalkingPapersAdapter.json
    ${PROJECT_SOURCE_DIR}/plugins/background/MWalkingPapersBackground/WalkingPapersAdapter.cpp
)
