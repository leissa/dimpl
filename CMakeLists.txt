cmake_minimum_required(VERSION 3.4.3 FATAL_ERROR)
project(dimpl VERSION 0.1 DESCRIPTION "a Dependently-typed IMPerative Langage")

option(PACKAGE_TESTS "Build the tests" ON)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED True)
set(CMAKE_CONFIGURATION_TYPES "Debug;Release" CACHE STRING "limited config" FORCE)
set(CMAKE_MODULE_PATH "${PROJECT_SOURCE_DIR}/cmake" ${CMAKE_MODULE_PATH})
set(NUM_THREADS 8 CACHE STRING "Number of threads for building dependencies at configure time.")

# Require out-of-source builds
file(TO_CMAKE_PATH "${PROJECT_BINARY_DIR}/CMakeLists.txt" LOC_PATH)
if(EXISTS "${LOC_PATH}")
    message(FATAL_ERROR "You cannot build in a source directory (or any directory with a CMakeLists.txt file). Please make a build subdirectory. Feel free to remove CMakeCache.txt and CMakeFiles.")
endif()

# Take care of version information
configure_file("${PROJECT_SOURCE_DIR}/include/dimpl/version.h.in" "${PROJECT_SOURCE_DIR}/include/dimpl/version.h")

# build and find dependencies
if(NOT EXISTS "${PROJECT_SOURCE_DIR}/thorin/build")
    file(MAKE_DIRECTORY "${PROJECT_SOURCE_DIR}/thorin/build")
    # configure
    execute_process(COMMAND ${CMAKE_COMMAND} -G "${CMAKE_GENERATOR}"
                                             -DCMAKE_BUILD_TYPE=${CMAKE_BUILD_TYPE}
                                             -DCMAKE_MAKE_PROGRAM=${CMAKE_MAKE_PROGRAM}
                                             -S "${PROJECT_SOURCE_DIR}/thorin"
                                             -B "${PROJECT_SOURCE_DIR}/thorin/build"
    )
    # build
    execute_process(COMMAND ${CMAKE_COMMAND} --build "${PROJECT_SOURCE_DIR}/thorin/build"
                                             -j ${NUM_THREADS}
                    WORKING_DIRECTORY "${PROJECT_SOURCE_DIR}/thorin"
    )
endif()

add_subdirectory(thorin)
add_subdirectory(src)
add_subdirectory(app)
if (PACKAGE_TESTS)
    add_subdirectory(gtest) # <- dimpl-unit-test binary
endif()
