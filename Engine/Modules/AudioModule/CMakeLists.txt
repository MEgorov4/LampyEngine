file(GLOB_RECURSE AUDIO_MODULE_SOURCE
    ${CMAKE_CURRENT_SOURCE_DIR}/*.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/*.h
)

format_source_group(${AUDIO_MODULE_SOURCE})
add_library(AudioModule STATIC ${AUDIO_MODULE_SOURCE})

find_package(VulkanLoader)
find_package(OpenAL)
target_link_libraries(AudioModule PUBLIC
    OpenAL::OpenAL
    LoggerModule
)