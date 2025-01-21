file(GLOB_RECURSE WINDOW_MODULE_SOURCE
    ${CMAKE_CURRENT_SOURCE_DIR}/*.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/*.h
)
format_source_group(${WINDOW_MODULE_SOURCE})

add_library(WindowModule STATIC ${WINDOW_MODULE_SOURCE})
find_package(glfw3)
find_package(VulkanLoader)
target_link_libraries(WindowModule PUBLIC
    glfw
    Vulkan::Loader
    LoggerModule
)