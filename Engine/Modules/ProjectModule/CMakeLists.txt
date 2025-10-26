file(GLOB_RECURSE PROJECT_MODULE_SOURCE ${CMAKE_CURRENT_SOURCE_DIR}/*.cpp
     ${CMAKE_CURRENT_SOURCE_DIR}/*.h)

format_source_group(${PROJECT_MODULE_SOURCE})

add_library(ProjectModule STATIC ${PROJECT_MODULE_SOURCE})

target_link_libraries(
  ProjectModule PRIVATE portable-file-dialogs::portable-file-dialogs
                        nlohmann_json::nlohmann_json boost::boost EngineContext)

