file(GLOB_RECURSE RENDER_MODULE_SOURCE
    ${CMAKE_CURRENT_SOURCE_DIR}/*.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/*.h
)

format_source_group(${RENDER_MODULE_SOURCE})
add_library(RenderModule STATIC ${RENDER_MODULE_SOURCE})

find_package(glfw3)
find_package(VulkanLoader)
find_package(glm)
target_link_libraries( RenderModule PUBLIC
    ObjectCoreModule
    ResourceModule
    LoggerModule
    PRIVATE
    ImGuiModule
    glfw
    Vulkan::Loader
    glm::glm
)