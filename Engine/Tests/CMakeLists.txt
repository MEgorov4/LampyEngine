set(TEST_PROJECT_NAME EngineTests)
file(GLOB_RECURSE TEST_SOURCES ${CMAKE_CURRENT_SOURCE_DIR}/*.cpp
     ${CMAKE_CURRENT_SOURCE_DIR}/*.h)

format_source_group(${TEST_SOURCES})
add_executable(${TEST_PROJECT_NAME} ${TEST_SOURCES})

# Find required packages
find_package(glm REQUIRED)
find_package(GTest REQUIRED)
find_package(nlohmann_json REQUIRED)
find_package(Boost REQUIRED)
find_package(tinyobjloader REQUIRED)
find_package(stb REQUIRED)
find_package(efsw REQUIRED)

# Link Engine library and all its dependencies
target_link_libraries(
  ${TEST_PROJECT_NAME}
  PUBLIC ${ENGINE_NAME}
         GTest::GTest
         GTest::Main
         nlohmann_json::nlohmann_json
         glm::glm
         Boost::boost
         tinyobjloader::tinyobjloader
         stb::stb
         efsw::efsw)

add_test(NAME ${TEST_PROJECT_NAME} COMMAND ${TEST_PROJECT_NAME})

include(GoogleTest)
gtest_discover_tests(${TEST_PROJECT_NAME})
