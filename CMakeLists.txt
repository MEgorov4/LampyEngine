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

# Function to format and group source files
function(format_source_group files)
    foreach(FILE ${${files}})
        get_filename_component(PATH "${FILE}" PATH)
        string(REPLACE "${CMAKE_CURRENT_SOURCE_DIR}/" "" GROUP "${PATH}")
        string(REPLACE "/" "\\" GROUP "${GROUP}")
        source_group("${GROUP}" FILES "${FILE}")
    endforeach()
endfunction()

# Automatically find all source and header files
file(GLOB_RECURSE SOURCES CONFIGURE_DEPENDS 
    ${CMAKE_CURRENT_SOURCE_DIR}/Source/Core/*.cpp 
    ${CMAKE_CURRENT_SOURCE_DIR}/Source/Core/*.h
)

file(GLOB_RECURSE SHADER_FILES CONFIGURE_DEPENDS 
    ${CMAKE_CURRENT_SOURCE_DIR}/Source/Core/Shaders/*.glsl
)

# ImGui backends
file(GLOB IMGUI_BACKENDS_SRC 
    external/imgui/backends/imgui_impl_glfw.cpp 
    external/imgui/backends/imgui_impl_opengl3.cpp
)

# Logger module
file(GLOB_RECURSE LOGGER_MODULE_SOURCE
    ${CMAKE_CURRENT_SOURCE_DIR}/Source/LoggerModule/*.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/Source/LoggerModule/*.h
)
# Render module
file(GLOB_RECURSE RENDER_MODULE_SOURCE
    ${CMAKE_CURRENT_SOURCE_DIR}/Source/RenderModule/*.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/Source/RenderModule/*.h
)

# Test sources
file(GLOB_RECURSE TEST_SOURCES CONFIGURE_DEPENDS 
    ${CMAKE_CURRENT_SOURCE_DIR}/Source/Tests/*.cpp 
    ${CMAKE_CURRENT_SOURCE_DIR}/Source/Tests/*.h
)

# Format source groups
format_source_group(SOURCES)
format_source_group(SHADER_FILES)
format_source_group(LOGGER_MODULE_SOURCE)
format_source_group(RENDER_MODULE_SOURCE)
format_source_group(TEST_SOURCES)

# Add logger module
add_library(LoggerModule STATIC ${LOGGER_MODULE_SOURCE})

# Add render module
add_library(RenderModule STATIC ${RENDER_MODULE_SOURCE})

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
    LoggerModule 
    RenderModule
    glfw 
    glm::glm 
    imgui::imgui
    tinyobjloader::tinyobjloader
    Vulkan::Loader
    nlohmann_json::nlohmann_json
    GTest::GTest
)

# Core library
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

# Test project
add_executable(${TEST_PROJECT_NAME} ${TEST_SOURCES})

target_link_libraries(${TEST_PROJECT_NAME}
    LoggerModule
    RenderModule
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
