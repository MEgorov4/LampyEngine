file(GLOB_RECURSE ENGINE_CONTEXT_SOURCE ${CMAKE_CURRENT_SOURCE_DIR}/*.cpp
     ${CMAKE_CURRENT_SOURCE_DIR}/*.h)

format_source_group(${ENGINE_CONTEXT_SOURCE})
add_library(EngineContext STATIC ${ENGINE_CONTEXT_SOURCE})

target_link_libraries(EngineContext PUBLIC glm::glm
Tracy::TracyClient)

# Подключаем precompiled header
target_precompile_headers(EngineContext PRIVATE EnginePCH.h)

target_include_directories(EngineContext
    PUBLIC
        ${CMAKE_CURRENT_SOURCE_DIR}
        ${CMAKE_SOURCE_DIR}/Engine     
)

option(LAMPY_ENABLE_TRACY "Enable Tracy profiler" ON)

if(LAMPY_ENABLE_TRACY)
    target_compile_definitions(EngineContext PRIVATE TRACY_ENABLE)
endif()