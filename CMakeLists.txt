cmake_minimum_required(VERSION 2.8.12)
project(balancer)

add_definitions("-std=c++17")

include(${CMAKE_BINARY_DIR}/conanbuildinfo_multi.cmake)
conan_basic_setup()

include_directories("${CMAKE_SOURCE_DIR}/includes")

file(GLOB_RECURSE HEADERS RELATIVE ${CMAKE_SOURCE_DIR} "includes/*.hpp")
file(GLOB_RECURSE SOURCES RELATIVE ${CMAKE_SOURCE_DIR} "sources/*.cpp")

set(CMAKE_SUPPRESS_REGENERATION true)

add_executable(balancer ${SOURCES} ${HEADERS})
conan_target_link_libraries(balancer)

