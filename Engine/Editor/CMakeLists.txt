file(GLOB_RECURSE EDITOR_MODULE_SOURCE
    ${CMAKE_CURRENT_SOURCE_DIR}/*.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/*.h
)

format_source_group(${EDITOR_MODULE_SOURCE})
add_library(EditorModule ${EDITOR_MODULE_SOURCE})

find_package(portable-file-dialogs)
target_link_libraries(EditorModule PUBLIC
    InputModule
    AudioModule
    WindowModule
    ObjectCoreModule
    LoggerModule
    RenderModule
    ResourceModule
    EngineContext
    EditorGuiModule
    portable-file-dialogs::portable-file-dialogs
    PRIVATE
    EventModule
)
