set(TEST_PROJECT_NAME EngineTests)
file(GLOB_RECURSE TEST_SOURCES
    ${CMAKE_CURRENT_SOURCE_DIR}/*.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/*.h
)

format_source_group(${TEST_SOURCES})
add_executable(${TEST_PROJECT_NAME} ${TEST_SOURCES})


find_package(glm)
find_package(gtest)
target_link_libraries(${TEST_PROJECT_NAME} PUBLIC
    InputModule
    AudioModule
    WindowModule
    LoggerModule
    ResourceModule
    RenderModule
    ObjectCoreModule
    glm::glm 
    GTest::GTest
    GTest::Main
)

add_test(NAME ${TEST_PROJECT_NAME} COMMAND ${TEST_PROJECT_NAME})

include(GoogleTest)
gtest_discover_tests(${TEST_PROJECT_NAME})
