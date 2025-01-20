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

# Engine executable
file(GLOB_RECURSE ENGINE_SOURCES CONFIGURE_DEPENDS 
    ${CMAKE_CURRENT_SOURCE_DIR}/Source/Engine/*.cpp 
    ${CMAKE_CURRENT_SOURCE_DIR}/Source/Engine/*.h
)

# Shader files
file(GLOB_RECURSE SHADER_FILES CONFIGURE_DEPENDS 
    ${CMAKE_CURRENT_SOURCE_DIR}/Source/Shaders/*.glsl
)

# ImGui backends
file(GLOB IMGUI_BACKENDS_SRC 
    external/imgui/backends/imgui_impl_glfw.cpp 
    external/imgui/backends/imgui_impl_opengl3.cpp
)

# Window module
file(GLOB_RECURSE WINDOW_MODULE_SOURCE
    ${CMAKE_CURRENT_SOURCE_DIR}/Source/WindowModule/*.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/Source/WindowModule/*.h
)

# Window module
file(GLOB_RECURSE INPUT_MODULE_SOURCE
    ${CMAKE_CURRENT_SOURCE_DIR}/Source/InputModule/*.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/Source/InputModule/*.h
)

# Resource module
file(GLOB_RECURSE RESOURCE_MODULE_SOURCE
    ${CMAKE_CURRENT_SOURCE_DIR}/Source/ResourceModule/*.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/Source/ResourceModule/*.h
)

# Audio module
file(GLOB_RECURSE AUDIO_MODULE_SOURCE
    ${CMAKE_CURRENT_SOURCE_DIR}/Source/AudioModule/*.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/Source/AudioModule/*.h
)

# Object core module
file(GLOB_RECURSE OBJECT_CORE_MODULE_SOURCE
    ${CMAKE_CURRENT_SOURCE_DIR}/Source/ObjectCoreModule/*.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/Source/ObjectCoreModule/*.h
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
file(GLOB_RECURSE TEST_SOURCES 
    ${CMAKE_CURRENT_SOURCE_DIR}/Source/Test/*.cpp 
    ${CMAKE_CURRENT_SOURCE_DIR}/Source/Test/*.h
)

# Format source groups
format_source_group(ENGINE_SOURCES)
format_source_group(SHADER_FILES)
format_source_group(WINDOW_MODULE_SOURCE)
format_source_group(INPUT_MODULE_SOURCE)
format_source_group(RESOURCE_MODULE_SOURCE)
format_source_group(AUDIO_MODULE_SOURCE)
format_source_group(OBJECT_CORE_MODULE_SOURCE)
format_source_group(LOGGER_MODULE_SOURCE)
format_source_group(RENDER_MODULE_SOURCE)
format_source_group(TEST_SOURCES)

# Find external libraries
find_package(glfw3 REQUIRED)
find_package(VulkanLoader REQUIRED)
find_package(glm REQUIRED)
find_package(imgui REQUIRED)
find_package(OpenAL)
find_package(tinyobjloader REQUIRED)
find_package(nlohmann_json REQUIRED)
find_package(GTest REQUIRED)

# Add window module
add_library(WindowModule STATIC ${WINDOW_MODULE_SOURCE})

# Add resource module
add_library(ResourceModule STATIC ${RESOURCE_MODULE_SOURCE})

# Add input module
add_library(InputModule STATIC ${INPUT_MODULE_SOURCE})

# Add audio module
add_library(AudioModule STATIC ${AUDIO_MODULE_SOURCE})

# Add object core module
add_library(ObjectCoreModule STATIC ${OBJECT_CORE_MODULE_SOURCE})

# Add logger module
add_library(LoggerModule STATIC ${LOGGER_MODULE_SOURCE})

# Add render module
add_library(RenderModule STATIC ${RENDER_MODULE_SOURCE})

# Add all files to the executable
add_executable(${PROJECT_NAME} ${ENGINE_SOURCES} ${SHADER_FILES} ${IMGUI_BACKENDS_SRC})

# window module link dependent
target_link_libraries(WindowModule PUBLIC
    glfw
    Vulkan::Loader
    LoggerModule
)

# input module lik dependent
target_link_libraries(InputModule PUBLIC
    glfw
    WindowModule
    LoggerModule
)

# resource module link dependent
target_link_libraries(ResourceModule PUBLIC
    glm::glm
    LoggerModule
)

# audio module link dependent
target_link_libraries(AudioModule PUBLIC
    OpenAL::OpenAL
    LoggerModule
)

# object core module link dependent
target_link_libraries(ObjectCoreModule PUBLIC
    nlohmann_json::nlohmann_json
    glm::glm
    LoggerModule
)

# Render module link dependent
target_link_libraries(RenderModule PUBLIC
    glfw
    Vulkan::Loader
    glm::glm
    ObjectCoreModule
    LoggerModule
)

# Link libraries
target_link_libraries(${PROJECT_NAME} PUBLIC
    InputModule
    AudioModule
    WindowModule
    ObjectCoreModule
    LoggerModule 
    RenderModule
    ResourceModule
    Vulkan::Loader
    nlohmann_json::nlohmann_json
    GTest::GTest
)


# Test project
add_executable(${TEST_PROJECT_NAME} ${TEST_SOURCES})

target_link_libraries(${TEST_PROJECT_NAME} PUBLIC
    InputModule
    AudioModule
    WindowModule
    LoggerModule
    ResourceModule
    RenderModule
    ObjectCoreModule
    glm::glm 
    GTest::GTest
    GTest::Main
)

add_test(NAME ${TEST_PROJECT_NAME} COMMAND ${TEST_PROJECT_NAME})

# Automatically discover tests in GTest
include(GoogleTest)
gtest_discover_tests(${TEST_PROJECT_NAME})
