file(GLOB_RECURSE TIME_MODULE_SOURCE ${CMAKE_CURRENT_SOURCE_DIR}/*.cpp
     ${CMAKE_CURRENT_SOURCE_DIR}/*.h)

format_source_group(${TIME_MODULE_SOURCE})
add_library(TimeModule STATIC ${TIME_MODULE_SOURCE})

target_link_libraries(
  TimeModule
  PUBLIC EngineContext
  PRIVATE SDL3::SDL3)

