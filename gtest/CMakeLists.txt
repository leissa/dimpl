enable_testing()
include(GoogleTest)

add_subdirectory("${PROJECT_SOURCE_DIR}/thorin/googletest" "googletest")

# keep cache clean
mark_as_advanced(
    BUILD_GMOCK BUILD_GTEST BUILD_SHARED_LIBS
    gmock_build_tests gtest_build_samples gtest_build_tests
    gtest_disable_pthreads gtest_force_shared_crt gtest_hide_internal_symbols
)

# keep IDEs folders clean
set_target_properties(gtest PROPERTIES FOLDER extern)
set_target_properties(gtest_main PROPERTIES FOLDER extern)
set_target_properties(gmock PROPERTIES FOLDER extern)
set_target_properties(gmock_main PROPERTIES FOLDER extern)

add_executable(dimpl-gtest
    test.cpp
    lexer.cpp
    parser.cpp
)

target_include_directories(dimpl-gtest PRIVATE ${PROJECT_SOURCE_DIR}/src ${PROJECT_SOURCE_DIR}/thorin/half/include)
target_link_libraries(dimpl-gtest gtest_main libdimpl)

gtest_discover_tests(dimpl-gtest TEST_PREFIX "dimpl.")
