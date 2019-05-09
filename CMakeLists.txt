if (${CMAKE_SOURCE_DIR} STREQUAL ${CMAKE_BINARY_DIR})
    message(FATAL_ERROR "Prevented in-tree build. Please create a build directory outside of the source code and call cmake from there")
endif ()

cmake_minimum_required(VERSION 3.9)
set(CMAKE_POSITION_INDEPENDENT_CODE ON)
set(CMAKE_EXPORT_COMPILE_COMMANDS ON)
set(CMAKE_CXX_STANDARD 17)

##! Project
project(albinos)

##! Prerequisites CTEST
enable_testing()

file(GLOB_RECURSE SOURCES_SERVICE service/*.cpp)
file(GLOB_RECURSE SOURCES_LIB lib/*.cpp)

set(BUILD_TESTING ON)
add_subdirectory(vendor/uvw)

add_executable(service ${SOURCES_SERVICE})
find_library(sqlite3_lib NAMES sqlite3)
target_link_libraries(service PUBLIC uv_a ${sqlite3_lib} stdc++fs)
target_sources(service PRIVATE vendor/loguru/loguru.cpp)
target_include_directories(service PUBLIC vendor/uvw/src vendor/uvw/deps/libuv/include vendor/json/single_include/nlohmann vendor/sql/hdr vendor/doctest/doctest vendor/strong_type/include/ vendor/expected vendor/loguru)
add_library(${PROJECT_NAME} SHARED ${SOURCES_LIB})
set_property(TARGET ${PROJECT_NAME} PROPERTY POSITION_INDEPENDENT_CODE ON)
target_link_libraries(${PROJECT_NAME} uv_a stdc++fs)
target_include_directories(${PROJECT_NAME} PUBLIC vendor/uvw/src vendor/uvw/deps/libuv/include vendor/json/single_include/nlohmann)
add_subdirectory(tests)
