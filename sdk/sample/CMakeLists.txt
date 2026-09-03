cmake_minimum_required(VERSION 3.24)

set(CMAKE_MSVC_RUNTIME_LIBRARY "MultiThreaded$<$<CONFIG:Debug>:Debug>")

project(foo_localize_sample
    VERSION 1.0.0
    DESCRIPTION "Example foobar2000 component that calls foo_localize"
    LANGUAGES C CXX
)

if(NOT WIN32)
    message(FATAL_ERROR "This sample CMake project is Windows-only. On macOS, add src/foo_localize_sample.cpp to your existing component target and include ../foo_localize_api.h.")
endif()

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

set(_sdk_guess "${CMAKE_CURRENT_SOURCE_DIR}/../../../../foobar_lrc/foobar-sdk")
if(NOT DEFINED FOOBAR_SDK_ROOT OR FOOBAR_SDK_ROOT STREQUAL "")
    if(EXISTS "${_sdk_guess}/foobar2000/SDK/foobar2000.h")
        set(FOOBAR_SDK_ROOT "${_sdk_guess}")
    endif()
endif()
set(FOOBAR_SDK_ROOT "${FOOBAR_SDK_ROOT}" CACHE PATH "foobar2000 SDK root (folder that contains foobar2000/SDK/foobar2000.h)")

if(NOT EXISTS "${FOOBAR_SDK_ROOT}/foobar2000/SDK/foobar2000.h")
    message(FATAL_ERROR "foobar2000 SDK not found. Pass -DFOOBAR_SDK_ROOT=C:/path/to/foobar-sdk")
endif()

include("${CMAKE_CURRENT_SOURCE_DIR}/cmake/Foobar2000SDK.cmake")

add_library(foo_localize_sample SHARED
    src/foo_localize_sample.cpp
)

target_include_directories(foo_localize_sample PRIVATE
    "${CMAKE_CURRENT_SOURCE_DIR}/.."
    "${CMAKE_CURRENT_SOURCE_DIR}/src"
)

target_compile_definitions(foo_localize_sample PRIVATE
    UNICODE
    _UNICODE
    WIN32
    _WINDOWS
    _CRT_SECURE_NO_WARNINGS
    NOMINMAX
    SHARED_EXPORTS
    FOO_LOCALIZE_SAMPLE_VERSION="${PROJECT_VERSION}"
)

target_compile_options(foo_localize_sample PRIVATE
    /W3
    /fp:fast
    /utf-8
    /Zc:__cplusplus
    /permissive-
)

target_link_libraries(foo_localize_sample PRIVATE
    fb2k_component_client
    fb2k_sdk
    fb2k_shared
    fb2k_pfc
)

if(MSVC)
    target_link_options(foo_localize_sample PRIVATE /FORCE:MULTIPLE)
endif()

set_target_properties(foo_localize_sample PROPERTIES
    OUTPUT_NAME "foo_localize_sample"
    PREFIX ""
    SUFFIX ".dll"
    RUNTIME_OUTPUT_DIRECTORY "${CMAKE_BINARY_DIR}/bin"
    LIBRARY_OUTPUT_DIRECTORY "${CMAKE_BINARY_DIR}/bin"
    MSVC_RUNTIME_LIBRARY "MultiThreaded$<$<CONFIG:Debug>:Debug>"
)
