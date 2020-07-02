
function(MerkaartorAddPlugin)
    # Parse the arguments using CMake built-in function
    cmake_parse_arguments(PARSE_ARGV 0 PLUGIN "" "NAME" "SOURCES")

    add_library(${PLUGIN_NAME} SHARED ${PLUGIN_SOURCES})
    target_compile_options(${PLUGIN_NAME} PRIVATE ${COMPILE_OPTIONS})
    target_include_directories(${PLUGIN_NAME} PRIVATE 
        ${PROJECT_SOURCE_DIR}/interfaces
        # TODO: The rest of includes are necessary for ProjectionChooser.cpp
        # used in MGdalBackground and MGeoTiffBackground. This probably needs
        # to be fixed, probably by giving ProjectionChooser an interface?
        ${PROJECT_SOURCE_DIR}/src/Utils
        ${PROJECT_SOURCE_DIR}/src/Preferences
        ${PROJECT_SOURCE_DIR}/src/common
    )
    target_link_libraries(${PLUGIN_NAME} Qt5::Svg Qt5::Network Qt5::Xml Qt5::Core Qt5::Gui Qt5::Concurrent Qt5::PrintSupport Qt5::Widgets ${EXIV2_LIBRARIES} )
endfunction()


MerkaartorAddPlugin(NAME MGdalBackground SOURCES
    ${PROJECT_SOURCE_DIR}/plugins/background/MGdalBackground/GdalAdapter.cpp
    ${PROJECT_SOURCE_DIR}/src/Utils/ProjectionChooser.cpp
    #${PROJECT_SOURCE_DIR}/plugins/background/MGdalBackground/GdalAdapter.json
)

MerkaartorAddPlugin(NAME MGeoTiffBackground SOURCES
    ${PROJECT_SOURCE_DIR}/plugins/background/MGeoTiffBackground/GeoTiffAdapter.cpp
    ${PROJECT_SOURCE_DIR}/src/Utils/ProjectionChooser.cpp
    #${PROJECT_SOURCE_DIR}/plugins/background/MGeoTiffBackground/GeoTiffAdapter.json
)

MerkaartorAddPlugin(NAME MMsBingMapBackground SOURCES
    ${PROJECT_SOURCE_DIR}/plugins/background/MMsBingMapBackground/Resources.qrc
    ${PROJECT_SOURCE_DIR}/plugins/background/MMsBingMapBackground/msbingmapadapter.cpp
    ${PROJECT_SOURCE_DIR}/plugins/background/MMsBingMapBackground/mapadapter.cpp
)

# Plugin not maintained for a long time.
# MerkaartorAddPlugin(NAME MCadastreFranceBackground SOURCES
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

MerkaartorAddPlugin(NAME MWalkingPapersBackground SOURCES
    #${PROJECT_SOURCE_DIR}/plugins/background/MWalkingPapersBackground/WalkingPapersAdapter.json
    ${PROJECT_SOURCE_DIR}/plugins/background/MWalkingPapersBackground/WalkingPapersAdapter.cpp
)
