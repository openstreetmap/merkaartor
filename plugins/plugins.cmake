
set(PLUGIN_MMS_BING_MAP_BACKGROUND_SOURCES
    ${PROJECT_SOURCE_DIR}/plugins/background/MMsBingMapBackground/Resources.qrc
    ${PROJECT_SOURCE_DIR}/plugins/background/MMsBingMapBackground/msbingmapadapter.cpp
    ${PROJECT_SOURCE_DIR}/plugins/background/MMsBingMapBackground/mapadapter.cpp
    #./background/MMsBingMapBackground/msbingmapadapter.json
    #./background/MMsBingMapBackground/mapadapter.h
    #./background/MMsBingMapBackground/msbingmapadapter.h
)

add_library(MMsBingMapBackground SHARED ${PLUGIN_MMS_BING_MAP_BACKGROUND_SOURCES})
target_compile_options(MMsBingMapBackground PRIVATE ${COMPILE_OPTIONS})
target_include_directories(MMsBingMapBackground PRIVATE ${PROJECT_SOURCE_DIR}/interfaces)
target_link_libraries(MMsBingMapBackground Qt5::Svg Qt5::Network Qt5::Xml Qt5::Core Qt5::Gui Qt5::Concurrent Qt5::PrintSupport Qt5::Widgets ${EXIV2_LIBRARIES} )
