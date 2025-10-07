cmake_minimum_required(VERSION 3.27)
project(idacpp VERSION 1.0.0 LANGUAGES CXX)

# C++ Standard
set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

# Include IDA SDK bootstrap (only if not already included)
if(NOT TARGET ida_platform_settings)
    include($ENV{IDASDK}/ida-cmake/bootstrap.cmake)
    find_package(idasdk REQUIRED)
endif()

# idacpp is a header-only library
add_library(idacpp INTERFACE)
add_library(idacpp::idacpp ALIAS idacpp)

target_include_directories(idacpp INTERFACE
    $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/include>
)

# Link to IDA SDK if available
if(TARGET idasdk::idasdk)
    target_link_libraries(idacpp INTERFACE idasdk::idasdk)
endif()

# Build examples
option(IDACPP_BUILD_EXAMPLES "Build idacpp examples" OFF)

if(IDACPP_BUILD_EXAMPLES)
    add_subdirectory(examples)
endif()
