cmake_minimum_required(VERSION 3.21)
project(QalamIDE VERSION 3.6.0 LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 23)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_AUTOMOC ON)
set(CMAKE_AUTORCC ON)
set(CMAKE_AUTOUIC ON)

include(cmake/WindowsRuntime.cmake)

if(WIN32)
    enable_language(RC)
endif()

find_package(Qt6 REQUIRED COMPONENTS Widgets Gui Core Svg)

if(WIN32)
    # Qt 6.10 exposes private headers as separate CMake packages. QWindowKit
    # uses these headers for native event forwarding and high-DPI conversion.
    find_package(Qt6 REQUIRED COMPONENTS CorePrivate GuiPrivate WidgetsPrivate)
    include(FetchContent)
    # Some Windows Git installations default to Schannel even when the
    # current account has no credential context. Keep nested submodule fetches
    # reproducible by using Git's bundled OpenSSL backend for this configure.
    set(ENV{GIT_CONFIG_COUNT} 1)
    set(ENV{GIT_CONFIG_KEY_0} http.sslBackend)
    set(ENV{GIT_CONFIG_VALUE_0} openssl)
    set(QWINDOWKIT_BUILD_STATIC ON CACHE BOOL "" FORCE)
    set(QWINDOWKIT_BUILD_WIDGETS ON CACHE BOOL "" FORCE)
    set(QWINDOWKIT_BUILD_QUICK OFF CACHE BOOL "" FORCE)
    set(QWINDOWKIT_BUILD_EXAMPLES OFF CACHE BOOL "" FORCE)
    set(QWINDOWKIT_BUILD_DOCUMENTATIONS OFF CACHE BOOL "" FORCE)
    set(QWINDOWKIT_ENABLE_STYLE_AGENT OFF CACHE BOOL "" FORCE)
    set(QWINDOWKIT_INSTALL OFF CACHE BOOL "" FORCE)
    FetchContent_Declare(qwindowkit
        GIT_REPOSITORY https://github.com/stdware/qwindowkit.git
        GIT_TAG 35e88f3655720ed0537c7dd1dde243bf4a70c94c
        GIT_SHALLOW TRUE
        GIT_SUBMODULES_RECURSE TRUE
        GIT_CONFIG http.sslBackend=openssl
    )
    FetchContent_MakeAvailable(qwindowkit)
endif()

option(QALAM_BUILD_TESTS "Build Qalam IDE tests" OFF)
if(QALAM_BUILD_TESTS)
    find_package(Qt6 REQUIRED COMPONENTS Test)
    enable_testing()
endif()

add_subdirectory(source)
add_subdirectory(qalam)

if(QALAM_BUILD_TESTS)
    add_subdirectory(tests)
endif()
