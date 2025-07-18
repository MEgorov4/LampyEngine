cmake_minimum_required(VERSION 3.21)

project(LampyEngine VERSION 1.0 LANGUAGES CXX)

enable_testing()

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

set(CMAKE_RUNTIME_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/Engine/Core)
set(CMAKE_ARCHIVE_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/Engine/Core)
set(CMAKE_LIBRARY_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/Engine/Core)

find_package(VulkanLoader REQUIRED)
find_package(glew REQUIRED)
find_package(glfw3 REQUIRED)
find_package(SDL3 REQUIRED)
find_package(OpenAL REQUIRED)
find_package(glm REQUIRED)
find_package(imgui REQUIRED)
find_package(nlohmann_json REQUIRED)
find_package(tinyobjloader REQUIRED)
find_package(GTest REQUIRED)
find_package(flecs REQUIRED)
find_package(Boost REQUIRED)
find_package(sol2 REQUIRED)
find_package(portable-file-dialogs REQUIRED)
find_package(fltk REQUIRED)
find_package(stb REQUIRED)
find_package(clip REQUIRED)
find_package(Bullet REQUIRED)

function(format_source_group)
    foreach(_source IN ITEMS ${ARGN})
        if (IS_ABSOLUTE "${_source}")
            file(RELATIVE_PATH _source_rel "${CMAKE_CURRENT_SOURCE_DIR}" "${_source}")
        else()
            set(_source_rel "${_source}")
        endif()
        get_filename_component(_source_path "${_source_rel}" PATH)
        string(REPLACE "/" "\\" _source_path_msvc "${_source_path}")
        source_group("${_source_path_msvc}" FILES "${_source}")
    endforeach()
endfunction(format_source_group)


if (MSVC)
    add_compile_options(/bigobj)
endif()

add_subdirectory(Engine)

