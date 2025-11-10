file(GLOB_RECURSE ${EDITOR_NAME}_MODULE_SOURCE ${CMAKE_CURRENT_SOURCE_DIR}/*.cpp
     ${CMAKE_CURRENT_SOURCE_DIR}/*.h)

format_source_group(${${EDITOR_NAME}_MODULE_SOURCE})
add_executable(${EDITOR_NAME} ${${EDITOR_NAME}_MODULE_SOURCE})

target_link_libraries(
  ${EDITOR_NAME}
  PRIVATE 
  ${ENGINE_NAME}
)

copy_resources_to_build(${EDITOR_NAME})