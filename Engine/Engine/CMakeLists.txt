file(GLOB_RECURSE ENGINE_SOURCE ${CMAKE_CURRENT_SOURCE_DIR}/*.cpp ${CMAKE_CURRENT_SOURCE_DIR}/*.h)

format_source_group(${ENGINE_SOURCE})
add_library(${ENGINE_NAME} STATIC ${ENGINE_SOURCE})

target_link_libraries(
  ${ENGINE_NAME}
   PUBLIC
    GLEW::GLEW 
    sdl::sdl  
    tinyobjloader::tinyobjloader  
    flecs::flecs_static 
    boost::boost 
    sol2::sol2 
    fltk::fltk 
    stb::stb 
    clip::clip 
    Bullet::Bullet 
    gtest::gtest 
    efsw::efsw 
    imgui::imgui
    nlohmann_json::nlohmann_json 
    glm::glm
    Tracy::TracyClient
    nuklear::nuklear)
target_precompile_headers(${ENGINE_NAME} PRIVATE EnginePCH.h)

target_include_directories(${ENGINE_NAME}
    PUBLIC
        $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}>
        $<BUILD_INTERFACE:${CMAKE_SOURCE_DIR}/Engine>
        $<INSTALL_INTERFACE:include/Engine>
)

option(LAMPY_ENABLE_TRACY "Enable Tracy profiler" ON)

if(LAMPY_ENABLE_TRACY)
    target_compile_definitions(${ENGINE_NAME} PRIVATE TRACY_ENABLE)
endif()