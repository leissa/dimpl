add_executable(dimpl main.cpp ${BACKEND})

target_include_directories(libdimpl PUBLIC ${Thorin_INCLUDE_DIRS} ${CMAKE_CURRENT_SOURCE_DIR}/../include)
target_compile_definitions(dimpl PUBLIC -DDIMPL_VERSION_MAJOR=${PROJECT_VERSION_MAJOR} -DDIMPL_VERSION_MINOR=${PROJECT_VERSION_MINOR})
target_link_libraries(dimpl PUBLIC libdimpl)

if (Thorin_HAS_LLVM_SUPPORT)
    target_compile_definitions(dimpl PUBLIC -DENABLE_LLVM)
    llvm_config(dimpl ${AnyDSL_LLVM_LINK_SHARED} core)
endif ()
