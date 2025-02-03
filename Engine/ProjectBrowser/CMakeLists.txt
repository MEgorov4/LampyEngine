file(GLOB_RECURSE PROJECT_BROWSER_SOURCE
    ${CMAKE_CURRENT_SOURCE_DIR}/*.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/*.h
)

format_source_group(${PROJECT_BROWSER_SOURCE})

add_executable(ProjectBrowser ${PROJECT_BROWSER_SOURCE})
find_package(nlohmann_json)
find_package(fltk)
target_link_libraries(ProjectBrowser PRIVATE fltk::fltk nlohmann_json::nlohmann_json)