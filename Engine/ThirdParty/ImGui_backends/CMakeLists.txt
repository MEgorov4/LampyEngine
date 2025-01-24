file(GLOB_RECURSE IMGUI_BACKENDS_SOURCE
    ${CMAKE_CURRENT_SOURCE_DIR}/*.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/*.h
)

format_source_group(${IMGUI_BACKENDS_SOURCE})

add_library(ImGuiBackends ${IMGUI_BACKENDS_SOURCE})
find_package(imgui)
find_package(glfw3)
find_package(VulkanLoader)
target_link_libraries(ImGuiBackends PRIVATE
imgui::imgui
Vulkan::Loader
glfw
)
