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

include(FetchContent)

###UVW
set(BUILD_TESTING ON)
FetchContent_Declare(
        uvw
        GIT_REPOSITORY https://github.com/skypjack/uvw
        GIT_TAG        v1.16.0_libuv-v1.28
)

FetchContent_Declare(
        replxx
        GIT_REPOSITORY https://github.com/AmokHuginnsson/replxx
)


FetchContent_MakeAvailable(uvw)

FetchContent_MakeAvailable(replxx)

add_library(uvw INTERFACE)
target_link_libraries(uvw INTERFACE uv_a)
target_include_directories(uvw INTERFACE ${uvw_SOURCE_DIR}/src ${uvw_SOURCE_DIR}/deps/libuv/include)
add_library(albinos::uvw ALIAS uvw)
###

add_executable(service ${SOURCES_SERVICE})
find_library(sqlite3_lib NAMES sqlite3)
target_link_libraries(service PUBLIC albinos::uvw ${sqlite3_lib} stdc++fs)
target_sources(service PRIVATE vendor/loguru/loguru.cpp)
target_include_directories(service PUBLIC vendor/json/single_include/nlohmann vendor/sql/hdr vendor/doctest/doctest vendor/strong_type/include/ vendor/expected vendor/loguru)
add_library(${PROJECT_NAME} SHARED ${SOURCES_LIB})
set_property(TARGET ${PROJECT_NAME} PROPERTY POSITION_INDEPENDENT_CODE ON)
target_link_libraries(${PROJECT_NAME} albinos::uvw stdc++fs)
target_include_directories(${PROJECT_NAME} PUBLIC vendor/uvw/src vendor/uvw/deps/libuv/include vendor/json/single_include/nlohmann)
add_subdirectory(tests)
