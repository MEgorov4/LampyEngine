file(GLOB_RECURSE MEMORY_MODULE_SOURCE
    ${CMAKE_CURRENT_SOURCE_DIR}/*.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/*.h
)

format_source_group(${MEMORY_MODULE_SOURCE})
add_library(MemoryModule STATIC ${MEMORY_MODULE_SOURCE})

target_link_libraries(MemoryModule PUBLIC
    LoggerModule
    EngineContext
    PRIVATE
    ResourceModule
    
)