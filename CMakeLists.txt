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

# UTF 8 support
if (MSVC)
    target_compile_options(psygine PUBLIC /utf-8)
endif ()

# NOMINMAX and WIN32 lean and mean for windows
if (MSVC)
    target_compile_definitions(psyengine PUBLIC
            NOMINMAX
            WIN32_LEAN_AND_MEAN
    )
endif ()

# Warnings (per-compiler)
if (MSVC)
    target_compile_options(psygine PRIVATE /W3 /permissive- /Zc:__cplusplus)
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

# bgfx
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
        FOLDER "Libraries" # shows up as 'Libraries/psygine' in IDEs
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

    # Collect and install all license/notice files
    set(LICENSE_INSTALL_DIR "${CMAKE_INSTALL_DATAROOTDIR}/licenses/${PROJECT_NAME}")

    set(ALL_LICENSE_FILES "")

    # Root LICENSE
    if (EXISTS "${CMAKE_CURRENT_SOURCE_DIR}/LICENSE")
        list(APPEND ALL_LICENSE_FILES "${CMAKE_CURRENT_SOURCE_DIR}/LICENSE")
    endif ()

    # Common third-party/notice filenames at repo root
    foreach (f IN ITEMS
            THIRD_PARTY_LICENSES.md THIRD_PARTY_NOTICES
            THIRD_PARTY.md THIRD_PARTY
            NOTICE NOTICES)
        if (EXISTS "${CMAKE_CURRENT_SOURCE_DIR}/${f}")
            list(APPEND ALL_LICENSE_FILES "${CMAKE_CURRENT_SOURCE_DIR}/${f}")
        endif ()
    endforeach ()

    # Everything under LICENSES/ (recommended to store third-party license texts)
    if (EXISTS "${CMAKE_CURRENT_SOURCE_DIR}/LICENSES")
        file(GLOB_RECURSE PROJECT_LICENSE_FILES
                "${CMAKE_CURRENT_SOURCE_DIR}/LICENSES/*")
        list(APPEND ALL_LICENSE_FILES ${PROJECT_LICENSE_FILES})
    endif ()

    # Try to include licenses from FetchContent bgfx stack if available
    # (bgfx.cmake brings bgfx, bx, bimg)
    set(_candidate_roots
            "${bgfx_SOURCE_DIR}"
            "${bgfx_SOURCE_DIR}/bgfx"
            "${bgfx_SOURCE_DIR}/bx"
            "${bgfx_SOURCE_DIR}/bimg")
    foreach (root IN LISTS _candidate_roots)
        if (EXISTS "${root}")
            file(GLOB _cand
                    "${root}/LICENSE" "${root}/LICENSE.*"
                    "${root}/COPYING*" "${root}/COPYRIGHT*")
            list(APPEND ALL_LICENSE_FILES ${_cand})
        endif ()
    endforeach ()

    # Try to include SDL3 license if available via the CONFIG package
    set(_sdl3_candidate_roots "")
    # 1) If SDL3_DIR (from find_package CONFIG) is known, search under it
    if (DEFINED SDL3_DIR AND EXISTS "${SDL3_DIR}")
        list(APPEND _sdl3_candidate_roots "${SDL3_DIR}")
        # Common install roots relative to config dir
        list(APPEND _sdl3_candidate_roots
                "${SDL3_DIR}/.."                   # package root
                "${SDL3_DIR}/../.."               # prefix root
                "${SDL3_DIR}/../../.."            # fallback up
        )
    endif ()

    # 2) Derive from include directories of SDL3::SDL3 target
    if (TARGET SDL3::SDL3)
        get_target_property(_sdl3_includes SDL3::SDL3 INTERFACE_INCLUDE_DIRECTORIES)
        if (_sdl3_includes)
            foreach (inc IN LISTS _sdl3_includes)
                if (EXISTS "${inc}")
                    list(APPEND _sdl3_candidate_roots
                            "${inc}"
                            "${inc}/.."            # include -> prefix root
                            "${inc}/../.."         # include/SDL3 -> prefix root
                            "${inc}/../../.."      # fallback up
                    )
                endif ()
            endforeach ()
        endif ()
        # Also try the imported location if present (may be empty for INTERFACE libs)
        get_target_property(_sdl3_libloc SDL3::SDL3 IMPORTED_LOCATION)
        if (_sdl3_libloc)
            get_filename_component(_sdl3_libdir "${_sdl3_libloc}" DIRECTORY)
            list(APPEND _sdl3_candidate_roots
                    "${_sdl3_libdir}"
                    "${_sdl3_libdir}/.."
                    "${_sdl3_libdir}/../..")
        endif ()
    endif ()

    if (_sdl3_candidate_roots)
        list(REMOVE_DUPLICATES _sdl3_candidate_roots)
        foreach (root IN LISTS _sdl3_candidate_roots)
            if (EXISTS "${root}")
                file(GLOB _sdl3_licenses
                        "${root}/LICENSE" "${root}/LICENSE.*"
                        "${root}/COPYING*" "${root}/COPYRIGHT*"
                        "${root}/SDL3_LICENSE*" "${root}/docs/README-SDL*.md"
                        "${root}/share/doc/SDL3/*LICENSE*")
                list(APPEND ALL_LICENSE_FILES ${_sdl3_licenses})
            endif ()
        endforeach ()
    endif ()

    list(REMOVE_DUPLICATES ALL_LICENSE_FILES)

    install(FILES ${ALL_LICENSE_FILES}
            DESTINATION "${LICENSE_INSTALL_DIR}"
    )


endif ()
