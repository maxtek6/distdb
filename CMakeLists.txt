cmake_minimum_required(VERSION 3.20)

if(NOT VCPKG_ROOT)
    set(VCPKG_ROOT $ENV{VCPKG_ROOT})
    if(NOT VCPKG_ROOT)
        message(FATAL_ERROR "VCPKG_ROOT environment variable is not set. Please set it to the root of your vcpkg installation.")
    endif()
endif()

set(CMAKE_TOOLCHAIN_FILE "${VCPKG_ROOT}/scripts/buildsystems/vcpkg.cmake" CACHE STRING "Vcpkg toolchain file")

project(distdb)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

find_package(argparse CONFIG REQUIRED)
find_package(flatcc CONFIG REQUIRED)
find_package(flatbuffers CONFIG REQUIRED)
find_package(Libevent CONFIG REQUIRED)
find_package(mio CONFIG REQUIRED)
find_package(xxHash CONFIG REQUIRED)

# Core library and compiler
set(DISTDB_SCHEMA_FILE "${CMAKE_CURRENT_SOURCE_DIR}/src/schema/schema.fbs")
set(DISTDB_INCLUDE_DIR ${CMAKE_CURRENT_SOURCE_DIR})
add_executable(distdbc)
add_subdirectory(src)

# C/C++ examples
add_subdirectory(examples)