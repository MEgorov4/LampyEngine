file(GLOB_RECURSE LOGGER_MODULE_SOURCE
    ${CMAKE_CURRENT_SOURCE_DIR}/*.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/*.h
)
format_source_group(${LOGGER_MODULE_SOURCE})
add_library(LoggerModule STATIC ${LOGGER_MODULE_SOURCE})

target_link_libraries(LoggerModule PRIVATE EventModule ProjectModule FilesystemModule)

