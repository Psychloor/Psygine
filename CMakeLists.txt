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
        src/library.cpp
        src/core/runtime.cpp

        src/core/time.cpp
        src/core/clock.cpp
)

set(PROJECT_HEADERS
        src/library.h
        src/core/runtime.hpp
        src/core/sdl_raii.hpp

        src/core/time.hpp
        src/core/clock.hpp
)

# ---------------- LIBRARY ----------------
add_library(psygine STATIC)
set_target_properties(psygine PROPERTIES POSITION_INDEPENDENT_CODE ON)

target_sources(psygine
        PRIVATE ${PROJECT_SOURCES}
)

target_sources(psygine
        PUBLIC
        FILE_SET HEADERS TYPE HEADERS
        BASE_DIRS src
        FILES ${PROJECT_HEADERS}
)

# Modern way to enforce language standard
target_compile_features(psygine PUBLIC cxx_std_23)

# Warnings (per-compiler)
if (MSVC)
    target_compile_options(psygine PRIVATE /W4 /permissive- /Zc:__cplusplus)
else ()
    target_compile_options(psygine PRIVATE -Wall -Wextra -Wpedantic)
    if (CMAKE_CXX_COMPILER_ID MATCHES "Clang|GNU")
        target_compile_options(psygine PRIVATE -Wconversion -Wsign-conversion)
    endif ()
endif ()

if (ENABLE_WARNINGS_AS_ERRORS)
    if (MSVC)
        target_compile_options(psygine PRIVATE /WX)
    else ()
        target_compile_options(psygine PRIVATE -Werror)
    endif ()
endif ()

# Unity build (CMake >= 3.16)
if (ENABLE_UNITY)
    set_target_properties(psygine PROPERTIES UNITY_BUILD ON)
endif ()

# Optional sanitizers (Debug-only recommended)
if (ENABLE_SANITIZERS AND NOT MSVC)
    target_compile_options(psygine PRIVATE -fsanitize=address,undefined -fno-omit-frame-pointer)
    target_link_options(psygine PRIVATE -fsanitize=address,undefined)
endif ()


# Generate export header
include(GenerateExportHeader)
GENERATE_EXPORT_HEADER(psygine
        EXPORT_FILE_NAME ${CMAKE_CURRENT_BINARY_DIR}/include/psygine/psygine_export.h
)

# Correctly expose include dirs (build + install)
target_include_directories(psygine
        PUBLIC
        $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/src>
        $<BUILD_INTERFACE:${CMAKE_CURRENT_BINARY_DIR}/include>
        $<INSTALL_INTERFACE:${CMAKE_INSTALL_INCLUDEDIR}/psygine>

)

# ---------------- DEPENDENCIES ----------------
include(FetchContent)
FetchContent_Declare(
        bgfx
        GIT_REPOSITORY https://github.com/bkaradzic/bgfx.cmake.git
        GIT_TAG master
        GIT_SHALLOW FALSE
)
FetchContent_MakeAvailable(bgfx)


# SDL3
find_package(SDL3 CONFIG REQUIRED)

target_link_libraries(psygine
        PUBLIC
        SDL3::SDL3
        bgfx
        bx,
        bimg
)

# Create namespaced ALIAS for consumers and consistency with exports
add_library(psygine::psygine ALIAS psygine)

# Properties for versioning
set_target_properties(psygine PROPERTIES
        VERSION ${PROJECT_VERSION}
        SOVERSION ${PROJECT_VERSION_MAJOR}
        EXPORT_NAME psygine
        # FOLDER "Libraries" # shows up as 'Libraries/MyLib' in IDEs
)

# ---------------- INSTALL / EXPORT ----------------
include(GNUInstallDirs)

# Only install/export when this is the top-level project
if (PROJECT_IS_TOP_LEVEL)
    install(TARGETS psygine
            EXPORT psygineTargets
            FILE_SET HEADERS DESTINATION ${CMAKE_INSTALL_INCLUDEDIR}/psygine
            RUNTIME DESTINATION ${CMAKE_INSTALL_BINDIR}
            LIBRARY DESTINATION ${CMAKE_INSTALL_LIBDIR}
            ARCHIVE DESTINATION ${CMAKE_INSTALL_LIBDIR}
    )

    install(EXPORT psygineTargets
            NAMESPACE psygine::
            DESTINATION ${CMAKE_INSTALL_LIBDIR}/cmake/psygine
    )

    include(CMakePackageConfigHelpers)

    write_basic_package_version_file(
            "${CMAKE_CURRENT_BINARY_DIR}/psygineConfigVersion.cmake"
            VERSION ${PROJECT_VERSION}
            COMPATIBILITY SameMajorVersion
    )

    if (NOT EXISTS "${CMAKE_CURRENT_SOURCE_DIR}/cmake/psygineConfig.cmake.in")
        file(WRITE "${CMAKE_CURRENT_SOURCE_DIR}/cmake/psygineConfig.cmake.in"
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
            "${CMAKE_CURRENT_SOURCE_DIR}/cmake/psygineConfig.cmake.in"
            "${CMAKE_CURRENT_BINARY_DIR}/psygineConfig.cmake"
            INSTALL_DESTINATION ${CMAKE_INSTALL_LIBDIR}/cmake/psygine
    )

    # Install config + version files
    install(FILES
            "${CMAKE_CURRENT_BINARY_DIR}/psygineConfig.cmake"
            "${CMAKE_CURRENT_BINARY_DIR}/psygineConfigVersion.cmake"
            DESTINATION ${CMAKE_INSTALL_LIBDIR}/cmake/psygine
    )

    # Install generated export header to the public include tree
    install(FILES
            "${CMAKE_CURRENT_BINARY_DIR}/include/psygine/psygine_export.h"
            DESTINATION ${CMAKE_INSTALL_INCLUDEDIR}/psygine
    )

    # Install BGFX License
    install(FILES
            "${CMAKE_CURRENT_SOURCE_DIR}/bgfx-license"
            DESTINATION "${CMAKE_INSTALL_LIBDIR}/shared/bgfx-license"
    )

endif ()
