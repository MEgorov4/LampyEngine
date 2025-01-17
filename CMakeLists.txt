# General info
cmake_minimum_required(VERSION 3.29.2)
set(PROJECT_NAME LampyEngine)
set(TEST_PROJECT_NAME TEST)
project(${PROJECT_NAME} VERSION 1.0.0)
message("------------Project files generation for ${PROJECT_NAME}------------")

# Common options
set(CMAKE_RUNTIME_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/bin)
set_property(DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR} PROPERTY VS_STARTUP_PROJECT ${PROJECT_NAME})
set_property(GLOBAL PROPERTY USE_FOLDERS OFF)
set(CMAKE_SUPPRESS_REGENERATION TRUE)
enable_testing()

# C++ options
set(CMAKE_CXX_STANDARD 20)

# Automatically find all source and header files
file(GLOB_RECURSE SOURCES CONFIGURE_DEPENDS 
    ${CMAKE_CURRENT_SOURCE_DIR}/Source/Core/*.cpp 
    ${CMAKE_CURRENT_SOURCE_DIR}/Source/Core/*.h
)

file(GLOB_RECURSE SHADER_FILES CONFIGURE_DEPENDS 
    ${CMAKE_CURRENT_SOURCE_DIR}/Source/Core/Shaders/*.glsl
)

# Grouping source files into folders (Visual Studio support)
foreach(FILE ${SOURCES})
    get_filename_component(PATH "${FILE}" PATH)
    string(REPLACE "${CMAKE_CURRENT_SOURCE_DIR}/" "" GROUP "${PATH}")
    string(REPLACE "/" "\\" GROUP "${GROUP}")
    source_group("${GROUP}" FILES "${FILE}")
endforeach()

# Grouping shaders separately
source_group("Shaders" FILES ${SHADER_FILES})

# ImGui backends
file(GLOB IMGUI_BACKENDS_SRC 
    external/imgui/backends/imgui_impl_glfw.cpp 
    external/imgui/backends/imgui_impl_opengl3.cpp
)

file(GLOB_RECURSE TEST_SOURCES CONFIGURE_DEPENDS 
    ${CMAKE_CURRENT_SOURCE_DIR}/Source/Tests/*.cpp 
    ${CMAKE_CURRENT_SOURCE_DIR}/Source/Tests/*.h
)

foreach(FILE ${TEST_SOURCES})
    get_filename_component(PATH "${FILE}" PATH)
    string(REPLACE "${CMAKE_CURRENT_SOURCE_DIR}/" "" GROUP "${PATH}")
    string(REPLACE "/" "\\" GROUP "${GROUP}")
    source_group("${GROUP}" FILES "${FILE}")
endforeach()

# Add all files to the executable
add_executable(${PROJECT_NAME} ${SOURCES} ${SHADER_FILES} ${IMGUI_BACKENDS_SRC})

# Find libraries
find_package(glfw3 REQUIRED)
find_package(VulkanLoader REQUIRED)
find_package(glm REQUIRED)
find_package(imgui REQUIRED)
find_package(tinyobjloader REQUIRED)
find_package(nlohmann_json REQUIRED)
find_package(GTest REQUIRED)
# Link libraries
target_link_libraries(${PROJECT_NAME} PRIVATE 
    glfw 
    glm::glm 
    imgui::imgui
    tinyobjloader::tinyobjloader
    Vulkan::Loader
    nlohmann_json::nlohmann_json
    GTest::GTest
)

set(FILES_WITHOUT_MAIN ${SOURCES})
list(REMOVE_ITEM FILES_WITHOUT_MAIN "${CMAKE_CURRENT_SOURCE_DIR}/Source/Core/main.cpp")

add_library(Core STATIC ${FILES_WITHOUT_MAIN})
target_link_libraries(Core 
    glfw 
    glm::glm 
    imgui::imgui
    tinyobjloader::tinyobjloader
    Vulkan::Loader
    nlohmann_json::nlohmann_json
    GTest::GTest
)
#Test
add_executable(${TEST_PROJECT_NAME} ${TEST_SOURCES})

target_link_libraries(${TEST_PROJECT_NAME}
    Core
    glfw 
    glm::glm 
    imgui::imgui
    tinyobjloader::tinyobjloader
    Vulkan::Loader
    nlohmann_json::nlohmann_json
    GTest::GTest
    GTest::Main
)
add_test(NAME ${TEST_PROJECT_NAME} COMMAND ${TEST_PROJECT_NAME})

# Automatically discover tests in GTest
include(GoogleTest)
gtest_discover_tests(${TEST_PROJECT_NAME})