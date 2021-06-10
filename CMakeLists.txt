cmake_minimum_required(VERSION 3.4.3 FATAL_ERROR)
project(dimpl VERSION 0.1 DESCRIPTION "a Dependently-typed IMPerative Langage")

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED True)
set(CMAKE_CONFIGURATION_TYPES "Debug;Release" CACHE STRING "limited config" FORCE)
set(CMAKE_MODULE_PATH "${PROJECT_SOURCE_DIR}/cmake" ${CMAKE_MODULE_PATH})
set(CMAKE_ARCHIVE_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/lib)
set(CMAKE_LIBRARY_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/lib)
set(CMAKE_RUNTIME_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/bin)

configure_file("${PROJECT_SOURCE_DIR}/include/dimpl/version.h.in" "${PROJECT_SOURCE_DIR}/include/dimpl/version.h")

add_subdirectory(thorin)
add_subdirectory(src)
add_subdirectory(driver)
if (BUILD_TESTING)
    add_subdirectory(gtest)
endif()
