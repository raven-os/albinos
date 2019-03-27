if (${CMAKE_SOURCE_DIR} STREQUAL ${CMAKE_BINARY_DIR})
    message(FATAL_ERROR "Prevented in-tree build. Please create a build directory outside of the source code and call cmake from there")
endif ()

cmake_minimum_required(VERSION 3.9)
set(CMAKE_EXPORT_COMPILE_COMMANDS ON)
set(CMAKE_CXX_STANDARD 17)

##! Project
project(albinos)

##! Prerequisites CTEST
enable_testing()


#if (NOT DEFINED ENV{CLAWS_ROOT_DEV})
#    message(STATUS "using insalled version of claws")
#    find_package(claws CONFIG REQUIRED)
#else ()
#    message(STATUS "using development version of claws")
#    add_subdirectory($ENV{CLAWS_ROOT_DEV} build)
#endif ()

file(GLOB_RECURSE SOURCES_SERVICE service/*.cpp)
file(GLOB_RECURSE SOURCES_LIB lib/*.cpp)

set(BUILD_TESTING ON)
add_subdirectory(vendor/uvw)

add_executable(service ${SOURCES_SERVICE})
find_library(sqlite3_lib NAMES sqlite3)
target_link_libraries(service PUBLIC uv_a ${sqlite3_lib} stdc++fs)
target_sources(service PRIVATE vendor/loguru/loguru.cpp)
target_include_directories(service PUBLIC vendor/uvw/src vendor/uvw/deps/libuv/include vendor/json/single_include/nlohmann vendor/sql/hdr vendor/doctest/doctest vendor/strong_type/include/ vendor/expected vendor/loguru)
add_library(config SHARED ${SOURCES_LIB})
target_include_directories(config PUBLIC vendor/uvw/src vendor/uvw/deps/libuv/include vendor/json/single_include/nlohmann)
add_subdirectory(tests)
