cmake_minimum_required(VERSION 3.21)

include(GNUInstallDirs)
include(CMakePackageConfigHelpers)

set(ENGINE_SDK_EXPORT_NAME EngineSDKTargets)

# Install public headers
set(_engine_headers_dir ${CMAKE_SOURCE_DIR}/Engine/Engine)
install(
    DIRECTORY ${_engine_headers_dir}/
    DESTINATION ${CMAKE_INSTALL_INCLUDEDIR}/Engine
    FILES_MATCHING
        PATTERN "*.h"
        PATTERN "*.hpp"
        PATTERN "*.hh"
        PATTERN "*.hxx"
        PATTERN "*.inl"
)

# Install runtime headers if they exist
if(EXISTS ${CMAKE_SOURCE_DIR}/Engine/Runtime)
    install(
        DIRECTORY ${CMAKE_SOURCE_DIR}/Engine/Runtime/
        DESTINATION ${CMAKE_INSTALL_INCLUDEDIR}/Runtime
        FILES_MATCHING
            PATTERN "*.h"
            PATTERN "*.hpp"
            PATTERN "*.hh"
            PATTERN "*.hxx"
            PATTERN "*.inl"
    )
endif()

# Install resources (optional)
if(EXISTS ${CMAKE_SOURCE_DIR}/Resources)
    install(
        DIRECTORY ${CMAKE_SOURCE_DIR}/Resources/
        DESTINATION Resources
    )
endif()

# Install build targets
install(
    TARGETS ${ENGINE_NAME}
    EXPORT ${ENGINE_SDK_EXPORT_NAME}
    RUNTIME DESTINATION ${CMAKE_INSTALL_BINDIR}
    LIBRARY DESTINATION ${CMAKE_INSTALL_LIBDIR}
    ARCHIVE DESTINATION ${CMAKE_INSTALL_LIBDIR}
)

if(TARGET ${EDITOR_NAME})
    install(
        TARGETS ${EDITOR_NAME}
        RUNTIME DESTINATION ${CMAKE_INSTALL_BINDIR}
    )
endif()

if(DEFINED PROJECT_BROWSER_EXECUTABLE)
    install(
        PROGRAMS ${PROJECT_BROWSER_EXECUTABLE}
        DESTINATION ${CMAKE_INSTALL_BINDIR}
        RENAME ProjectBrowser${CMAKE_EXECUTABLE_SUFFIX}
    )
endif()

if(TARGET ${RUNTIME_NAME})
    install(
        TARGETS ${RUNTIME_NAME}
        RUNTIME DESTINATION ${CMAKE_INSTALL_BINDIR}
    )
endif()

# Export targets
install(
    EXPORT ${ENGINE_SDK_EXPORT_NAME}
    FILE ${ENGINE_SDK_EXPORT_NAME}.cmake
    NAMESPACE EngineSDK::
    DESTINATION cmake
)

configure_package_config_file(
    ${CMAKE_CURRENT_LIST_DIR}/EngineSDKConfig.cmake.in
    ${CMAKE_CURRENT_BINARY_DIR}/EngineSDKConfig.cmake
    INSTALL_DESTINATION cmake
)

install(FILES ${CMAKE_CURRENT_BINARY_DIR}/EngineSDKConfig.cmake DESTINATION cmake)

configure_file(${CMAKE_CURRENT_LIST_DIR}/version.txt.in ${CMAKE_CURRENT_BINARY_DIR}/version.txt @ONLY)
install(FILES ${CMAKE_CURRENT_BINARY_DIR}/version.txt DESTINATION .)


