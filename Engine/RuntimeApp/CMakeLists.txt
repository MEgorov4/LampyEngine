file(GLOB_RECURSE ${RUNTIME_NAME}_SOURCE
    ${CMAKE_CURRENT_SOURCE_DIR}/*.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/*.h
)

format_source_group(${${RUNTIME_NAME}_SOURCE})

add_executable(${RUNTIME_NAME} ${${RUNTIME_NAME}_SOURCE})

target_link_libraries(${RUNTIME_NAME}
    PRIVATE
        ${ENGINE_NAME}
)

# Copy Resources to build directory
copy_resources_to_build(${RUNTIME_NAME})

