set(EDITOR_PROJECT_NAME Editor)

file(GLOB_RECURSE EDITOR_SOURCE
    ${CMAKE_CURRENT_SOURCE_DIR}/*.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/*.h
)

format_source_group(${EDITOR_SOURCE})

add_executable(${EDITOR_PROJECT_NAME} ${EDITOR_SOURCE})

target_link_libraries(${EDITOR_PROJECT_NAME} PUBLIC
    InputModule
    AudioModule
    WindowModule
    ObjectCoreModule
    LoggerModule
    RenderModule
    ResourceModule
)