add_executable(dimpl main.cpp)

target_compile_definitions(dimpl PUBLIC -DDIMPL_VERSION_MAJOR=${PROJECT_VERSION_MAJOR} -DDIMPL_VERSION_MINOR=${PROJECT_VERSION_MINOR})
target_link_libraries(dimpl PUBLIC libdimpl thorin)
