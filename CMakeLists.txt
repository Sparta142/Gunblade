cmake_minimum_required(VERSION 3.15)
cmake_policy(SET CMP0091 NEW) # MSVC runtime library flags abstraction

# vcpkg configuration
set(VCPKG_TARGET_TRIPLET "x64-windows-static")
set(VCPKG_OVERLAY_PORTS "${CMAKE_CURRENT_SOURCE_DIR}/tools/custom-ports")

if(NOT DEFINED CMAKE_TOOLCHAIN_FILE)
    if(DEFINED ENV{VCPKG_ROOT})
        set(VCPKG_BASE "$ENV{VCPKG_ROOT}")
    else()
        set(VCPKG_BASE "${CMAKE_CURRENT_SOURCE_DIR}/tools/vcpkg")
    endif()

    set(CMAKE_TOOLCHAIN_FILE "${VCPKG_BASE}/scripts/buildsystems/vcpkg.cmake" CACHE STRING "")
endif()

project(
    Gunblade
    VERSION 0.1.0
    DESCRIPTION "A tool for monitoring FINAL FANTASY XIV network traffic."
    LANGUAGES CXX
)

# TODO: Support other platforms
if(NOT WIN32)
    message(FATAL_ERROR "Only building for Windows is supported")
endif()

add_subdirectory(src)
