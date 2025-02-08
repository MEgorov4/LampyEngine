file(GLOB_RECURSE ENGINE_SOURCE
    ${CMAKE_CURRENT_SOURCE_DIR}/*.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/*.h
)

format_source_group(${ENGINE_SOURCE})
add_executable(Engine ${ENGINE_SOURCE})

target_link_libraries(Engine PUBLIC
    InputModule
    AudioModule
    WindowModule
    LoggerModule
    RenderModule
    ResourceModule
    EditorModule
    EngineContext
    PRIVATE
    EventModule
    ObjectCoreModule
    LuaScriptModule
)