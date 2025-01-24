file(GLOB_RECURSE IMGUI_MODULE
    ${CMAKE_CURRENT_SOURCE_DIR}/*.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/*.h
)

format_source_group(${IMGUI_MODULE})

add_library(ImGuiModule ${IMGUI_MODULE})
find_package(imgui)


target_link_libraries(ImGuiModule PRIVATE
    imgui::imgui
    ImGuiBackends
    WindowModule
)