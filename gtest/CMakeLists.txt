add_executable(dimpl-gtest
    test.cpp
    lexer.cpp
    parser.cpp
)

target_link_libraries(dimpl-gtest PRIVATE gtest_main libdimpl)
gtest_discover_tests (dimpl-gtest TEST_PREFIX "dimpl.")
