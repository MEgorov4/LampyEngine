file(GLOB_RECURSE WINDOW_MODULE_SOURCE
    ${CMAKE_CURRENT_SOURCE_DIR}/*.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/*.h
)
format_source_group(${WINDOW_MODULE_SOURCE})

add_library(WindowModule STATIC ${WINDOW_MODULE_SOURCE})
target_link_libraries(WindowModule PUBLIC
    glfw
    sdl::sdl
    Vulkan::Loader
    LoggerModule
    GLEW::GLEW
    InputModule
    EngineContext
)