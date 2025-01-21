file(GLOB_RECURSE ENGINE_CORE_SOURCE
    ${CMAKE_CURRENT_SOURCE_DIR}/*.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/*.h
)

format_source_group(${ENGINE_CORE_SOURCE})
add_library(EngineCore STATIC ${ENGINE_CORE_SOURCE})

target_link_libraries(EngineCore PUBLIC
    WindowModule
    AudioModule
)