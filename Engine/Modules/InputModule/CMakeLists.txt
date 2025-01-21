file(GLOB_RECURSE INPUT_MODULE_SOURCE
    ${CMAKE_CURRENT_SOURCE_DIR}/*.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/*.h
)
format_source_group(${INPUT_MODULE_SOURCE})
add_library(InputModule STATIC ${INPUT_MODULE_SOURCE})
find_package(glfw3)
target_link_libraries(InputModule PRIVATE
    glfw
    WindowModule
    LoggerModule
)