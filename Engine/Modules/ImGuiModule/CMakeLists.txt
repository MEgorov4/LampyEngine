file(GLOB_RECURSE IMGUI_MODULE
    ${CMAKE_CURRENT_SOURCE_DIR}/*.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/*.h
)

format_source_group(${IMGUI_MODULE})

add_library(ImGuiModule ${IMGUI_MODULE})

target_link_libraries(ImGuiModule 
    PUBLIC 
        imgui::imgui
    PRIVATE
        glfw
        Vulkan::Loader
)