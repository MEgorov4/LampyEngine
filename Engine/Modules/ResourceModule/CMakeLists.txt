file(GLOB_RECURSE RESOURCE_MODULE_SOURCE
    ${CMAKE_CURRENT_SOURCE_DIR}/*.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/*.h
)

format_source_group(${RESOURCE_MODULE_SOURCE})

add_library(ResourceModule STATIC ${RESOURCE_MODULE_SOURCE})

target_link_libraries(ResourceModule PUBLIC
    glm::glm
    LoggerModule
    RenderModule
    stb::stb
    tinyobjloader::tinyobjloader
)