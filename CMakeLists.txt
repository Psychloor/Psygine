cmake_minimum_required(VERSION 4.0)

project(psygine
        VERSION 0.1.0
        LANGUAGES CXX
)

# ---------------- GLOBAL SETTINGS ----------------
# Enforce modern C++
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

set(CMAKE_EXPORT_COMPILE_COMMANDS ON)

# Build options
option(BUILD_SHARED_LIBS "Build shared libraries" ON)
option(ENABLE_WARNINGS_AS_ERRORS "Treat warnings as errors" OFF)
option(ENABLE_UNITY "Enable unity/jumbo builds for faster compilation" OFF)
option(ENABLE_SANITIZERS "Enable Address/Undefined sanitizers for Clang/GCC (non-MSVC)" OFF)

# Organize targets in IDEs (CLion, VS, Xcode, etc.)
set_property(GLOBAL PROPERTY USE_FOLDERS ON)

# ---------------- SOURCE COLLECTION ----------------
set(PROJECT_SOURCES
        src/psygine/core/runtime.cpp

        src/psygine/core/time.cpp
        src/psygine/core/clock.cpp
)

# Automatically collect public headers under src/psygine/**
file(GLOB_RECURSE PROJECT_HEADERS CONFIGURE_DEPENDS
        "${CMAKE_CURRENT_SOURCE_DIR}/src/psygine/**/*.hpp"
        "${CMAKE_CURRENT_SOURCE_DIR}/src/psygine/**/*.h"

)

source_group(TREE "${CMAKE_CURRENT_SOURCE_DIR}/src"
        FILES ${PROJECT_SOURCES} ${PROJECT_HEADERS})

# ---------------- LIBRARY ----------------
add_library(${PROJECT_NAME} STATIC)
set_target_properties(${PROJECT_NAME} PROPERTIES POSITION_INDEPENDENT_CODE ON)

target_sources(${PROJECT_NAME}
        PRIVATE ${PROJECT_SOURCES}
)

target_sources(${PROJECT_NAME}
        PUBLIC
        FILE_SET HEADERS TYPE HEADERS
        BASE_DIRS src
        FILES ${PROJECT_HEADERS}
)

# Modern way to enforce language standard
target_compile_features(${PROJECT_NAME} PUBLIC cxx_std_23)

# UTF 8 support
if (MSVC)
    target_compile_options(${PROJECT_NAME} PUBLIC /utf-8)
endif ()

# NOMINMAX and WIN32 lean and mean for windows
if (MSVC)
    target_compile_definitions(${PROJECT_NAME} PUBLIC
            NOMINMAX
            WIN32_LEAN_AND_MEAN
    )
endif ()

# Warnings (per-compiler)
if (MSVC)
    target_compile_options(${PROJECT_NAME} PRIVATE /W3 /permissive- /Zc:__cplusplus)
else ()
    target_compile_options(${PROJECT_NAME} PRIVATE -Wall -Wextra -Wpedantic)
    if (CMAKE_CXX_COMPILER_ID MATCHES "Clang|GNU")
        target_compile_options(${PROJECT_NAME} PRIVATE -Wconversion -Wsign-conversion)
    endif ()
endif ()

if (ENABLE_WARNINGS_AS_ERRORS)
    if (MSVC)
        target_compile_options(${PROJECT_NAME} PRIVATE /WX)
    else ()
        target_compile_options(${PROJECT_NAME} PRIVATE -Werror)
    endif ()
endif ()

# Unity build (CMake >= 3.16)
if (ENABLE_UNITY)
    set_target_properties(${PROJECT_NAME} PROPERTIES UNITY_BUILD ON)
endif ()

# Optional sanitizers (Debug-only recommended)
if (ENABLE_SANITIZERS AND NOT MSVC)
    target_compile_options(${PROJECT_NAME} PRIVATE -fsanitize=address,undefined -fno-omit-frame-pointer)
    target_link_options(${PROJECT_NAME} PRIVATE -fsanitize=address,undefined)
endif ()


# Generate export header
include(GenerateExportHeader)
GENERATE_EXPORT_HEADER(${PROJECT_NAME}
        EXPORT_FILE_NAME ${CMAKE_CURRENT_BINARY_DIR}/include/${PROJECT_NAME}/${PROJECT_NAME}_export.h
)

# Correctly expose include dirs (build + install)
target_include_directories(${PROJECT_NAME}
        PUBLIC
        $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/src>
        $<BUILD_INTERFACE:${CMAKE_CURRENT_BINARY_DIR}/include>
        $<INSTALL_INTERFACE:${CMAKE_INSTALL_INCLUDEDIR}>
)


# ---------------- DEPENDENCIES ----------------
include(FetchContent)

# bgfx
find_package(bgfx CONFIG QUIET)
if (NOT bgfx_FOUND)
    FetchContent_Declare(
            bgfx
            GIT_REPOSITORY https://github.com/bkaradzic/bgfx.cmake.git
            GIT_TAG master
            GIT_SHALLOW FALSE
    )
    FetchContent_MakeAvailable(bgfx)
endif ()

# SDL3
find_package(SDL3 CONFIG REQUIRED)

target_link_libraries(${PROJECT_NAME}
        PUBLIC
        SDL3::SDL3
        bgfx
        bx,
        bimg
)

# Create namespaced ALIAS for consumers and consistency with exports
add_library(${PROJECT_NAME}::${PROJECT_NAME} ALIAS ${PROJECT_NAME})

# Properties for versioning
set_target_properties(${PROJECT_NAME} PROPERTIES
        VERSION ${PROJECT_VERSION}
        SOVERSION ${PROJECT_VERSION_MAJOR}
        EXPORT_NAME ${PROJECT_NAME}
        FOLDER "Libraries" # shows up as 'Libraries/${PROJECT_NAME}' in IDEs
)

# ---------------- INSTALL / EXPORT ----------------
include(GNUInstallDirs)

# Only install/export when this is the top-level project
if (PROJECT_IS_TOP_LEVEL)
    install(TARGETS ${PROJECT_NAME}
            EXPORT ${PROJECT_NAME}Targets
            FILE_SET HEADERS DESTINATION ${CMAKE_INSTALL_INCLUDEDIR}
            RUNTIME DESTINATION ${CMAKE_INSTALL_BINDIR}
            LIBRARY DESTINATION ${CMAKE_INSTALL_LIBDIR}
            ARCHIVE DESTINATION ${CMAKE_INSTALL_LIBDIR}
    )

    install(EXPORT ${PROJECT_NAME}Targets
            NAMESPACE ${PROJECT_NAME}::
            DESTINATION ${CMAKE_INSTALL_LIBDIR}/cmake/${PROJECT_NAME}
    )

    include(CMakePackageConfigHelpers)

    write_basic_package_version_file(
            "${CMAKE_CURRENT_BINARY_DIR}/${PROJECT_NAME}ConfigVersion.cmake"
            VERSION ${PROJECT_VERSION}
            COMPATIBILITY SameMajorVersion
    )

    if (NOT EXISTS "${CMAKE_CURRENT_SOURCE_DIR}/cmake/${PROJECT_NAME}Config.cmake.in")
        file(WRITE "${CMAKE_CURRENT_SOURCE_DIR}/cmake/${PROJECT_NAME}Config.cmake.in"
                "@PACKAGE_INIT@

include(CMakeFindDependencyMacro)

# Uncomment and modify if package has dependencies
# find_dependency(fmt CONFIG REQUIRED)
find_dependency(SDL3 CONFIG REQUIRED)

include(\"\${CMAKE_CURRENT_LIST_DIR}/@PROJECT_NAME@Targets.cmake\")

check_required_components(@PROJECT_NAME@)
")
    endif ()

    configure_package_config_file(
            "${CMAKE_CURRENT_SOURCE_DIR}/cmake/${PROJECT_NAME}Config.cmake.in"
            "${CMAKE_CURRENT_BINARY_DIR}/${PROJECT_NAME}Config.cmake"
            INSTALL_DESTINATION ${CMAKE_INSTALL_LIBDIR}/cmake/${PROJECT_NAME}
    )

    # Install config + version files
    install(FILES
            "${CMAKE_CURRENT_BINARY_DIR}/${PROJECT_NAME}Config.cmake"
            "${CMAKE_CURRENT_BINARY_DIR}/${PROJECT_NAME}ConfigVersion.cmake"
            DESTINATION ${CMAKE_INSTALL_LIBDIR}/cmake/${PROJECT_NAME}
    )

    # Export header still goes under include/<project> (e.g., include/psygine/...)
    install(FILES
            "${CMAKE_CURRENT_BINARY_DIR}/include/${PROJECT_NAME}/${PROJECT_NAME}_export.h"
            DESTINATION ${CMAKE_INSTALL_INCLUDEDIR}/${PROJECT_NAME}
    )

    # Install all files under the LICENSES/ folder recursively, preserving structure
    if (EXISTS "${CMAKE_CURRENT_SOURCE_DIR}/LICENSES")
        install(DIRECTORY "${CMAKE_CURRENT_SOURCE_DIR}/LICENSES/"
                DESTINATION "${CMAKE_INSTALL_DATAROOTDIR}/licenses/${PROJECT_NAME}/LICENSES"
                FILES_MATCHING
                PATTERN "*"
        )
    endif ()


endif ()
