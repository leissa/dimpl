add_executable(dimpl-gtest
    test.cpp
    lexer.cpp
    parser.cpp
)

target_include_directories(dimpl-gtest PRIVATE ${PROJECT_SOURCE_DIR}/src ${PROJECT_SOURCE_DIR}/thorin/half/include)
target_link_libraries     (dimpl-gtest PRIVATE gtest_main libdimpl)
gtest_discover_tests      (dimpl-gtest TEST_PREFIX "dimpl.")
