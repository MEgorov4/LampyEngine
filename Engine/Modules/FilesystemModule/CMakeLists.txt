file(GLOB_RECURSE FILESYSTEM_MODULE
    ${CMAKE_CURRENT_SOURCE_DIR}/*cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/*h
)

format_source_group(${FILESYSTEM_MODULE})

add_library(FilesystemModule ${FILESYSTEM_MODULE})
target_link_libraries(FilesystemModule PRIVATE 
    LoggerModule 
    ProjectModule 
    clip::clip
    EngineContext
)