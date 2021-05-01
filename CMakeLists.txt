cmake_minimum_required(VERSION 3.4.3 FATAL_ERROR)
project(dimpl VERSION 0.1)
include(CTest)

set(CMAKE_CONFIGURATION_TYPES "Debug;Release" CACHE STRING "limited config" FORCE)
set(CMAKE_CXX_STANDARD 20)

find_package(Thorin REQUIRED)

if (BUILD_TESTING)
    add_subdirectory(unit-test)
endif()

add_subdirectory(src)
