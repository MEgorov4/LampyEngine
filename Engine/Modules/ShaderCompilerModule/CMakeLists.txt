file(GLOB_RECURSE SHADER_COMPILER_MODULE_SOURCE
     ${CMAKE_CURRENT_SOURCE_DIR}/*.cpp ${CMAKE_CURRENT_SOURCE_DIR}/*.h)

format_source_group(${SHADER_COMPILER_MODULE_SOURCE})
add_library(ShaderCompilerModule ${SHADER_COMPILER_MODULE_SOURCE})

target_link_libraries(
  ShaderCompilerModule
  PUBLIC EngineContext
  PRIVATE boost::boost)
