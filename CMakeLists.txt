cmake_minimum_required(VERSION 3.4.3 FATAL_ERROR)
project(dimpl VERSION 0.1 LANGUAGES CXX)

include(FetchContent)

set(CMAKE_CONFIGURATION_TYPES "Debug;Release" CACHE STRING "limited config" FORCE)
set(CMAKE_MODULE_PATH "${PROJECT_SOURCE_DIR}/cmake" ${CMAKE_MODULE_PATH})
set(CMAKE_CXX_STANDARD 20)

set(NUM_THREADS 8 CACHE STRING "Number of threads for building dependencies at configure time.")

# Require out-of-source builds
file(TO_CMAKE_PATH "${PROJECT_BINARY_DIR}/CMakeLists.txt" LOC_PATH)
if(EXISTS "${LOC_PATH}")
    message(FATAL_ERROR "You cannot build in a source directory (or any directory with a CMakeLists.txt file). Please make a build subdirectory. Feel free to remove CMakeCache.txt and CMakeFiles.")
endif()

# Take care of version information
configure_file("${PROJECT_SOURCE_DIR}/include/dimpl/version.h.in" "${PROJECT_SOURCE_DIR}/include/dimpl/version.h")

# Update submodules
find_package(Git QUIET)
if(GIT_FOUND AND EXISTS "${PROJECT_SOURCE_DIR}/.git")
# Update submodules as needed
    option(GIT_SUBMODULE "Check submodules during build" ON)
    if(GIT_SUBMODULE)
        message(STATUS "Submodule update")
        execute_process(COMMAND ${GIT_EXECUTABLE} submodule update --init --recursive
                        WORKING_DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR}
                        RESULT_VARIABLE GIT_SUBMOD_RESULT)
        if(NOT GIT_SUBMOD_RESULT EQUAL "0")
            message(FATAL_ERROR "git submodule update --init failed with ${GIT_SUBMOD_RESULT}, please checkout submodules")
        endif()
    endif()
endif()

if(NOT EXISTS "${PROJECT_SOURCE_DIR}/thorin/CMakeLists.txt")
    message(FATAL_ERROR "The submodules were not downloaded! GIT_SUBMODULE was turned off or failed. Please update submodules and try again.")
endif()

# build and find dependencies
if(NOT EXISTS "${PROJECT_SOURCE_DIR}/thorin/build")
    file(MAKE_DIRECTORY "${PROJECT_SOURCE_DIR}/thorin/build")
    # Configure
    execute_process(COMMAND ${CMAKE_COMMAND} -G "${CMAKE_GENERATOR}"
                                             -DCMAKE_BUILD_TYPE=${CMAKE_BUILD_TYPE}
                                             -DCMAKE_MAKE_PROGRAM=${CMAKE_MAKE_PROGRAM}
                                             -S "${PROJECT_SOURCE_DIR}/thorin"
                                             -B "${PROJECT_SOURCE_DIR}/thorin/build"
    )
    # Build
    execute_process(COMMAND ${CMAKE_COMMAND} --build "${PROJECT_SOURCE_DIR}/thorin/build"
                                             -j ${NUM_THREADS}
                    WORKING_DIRECTORY "${PROJECT_SOURCE_DIR}/thorin"
    )
endif()
find_package(Thorin REQUIRED PATHS ${PROJECT_SOURCE_DIR}/thorin/build/share/anydsl/cmake/)

add_subdirectory(src) # <- libdimpl
add_subdirectory(app) # <- dimpl binary
include(CTest)
if (BUILD_TESTING)
    add_subdirectory(unit-test) # <- dimpl-unit-test binary
endif()



