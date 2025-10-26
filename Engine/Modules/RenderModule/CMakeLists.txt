file(GLOB_RECURSE RENDER_MODULE_SOURCE ${CMAKE_CURRENT_SOURCE_DIR}/*.cpp
     ${CMAKE_CURRENT_SOURCE_DIR}/*.h)

format_source_group(${RENDER_MODULE_SOURCE})
add_library(RenderModule STATIC ${RENDER_MODULE_SOURCE})

target_link_libraries(
  RenderModule
  PUBLIC ObjectCoreModule ResourceModule
  PRIVATE GLEW::GLEW ImGuiModule EngineContext sdl::sdl)
