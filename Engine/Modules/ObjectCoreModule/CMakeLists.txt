file(GLOB_RECURSE OBJECT_CORE_MODULE_SOURCE
    ${CMAKE_CURRENT_SOURCE_DIR}/*.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/*.h
)

format_source_group(${OBJECT_CORE_MODULE_SOURCE})
add_library(ObjectCoreModule STATIC ${OBJECT_CORE_MODULE_SOURCE})

find_package(nlohmann_json REQUIRED)
find_package(glm REQUIRED)
find_package(flecs REQUIRED)
find_package(sol2 REQUIRED)
target_link_libraries(ObjectCoreModule PUBLIC
    nlohmann_json::nlohmann_json
    glm::glm
    LoggerModule
    ProjectModule
    flecs::flecs_static
)

target_link_libraries(ObjectCoreModule PUBLIC
    LuaScriptModule
)