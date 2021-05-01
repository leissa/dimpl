add_library(libdimpl
    ../include/dimpl/ast.h      stream.cpp
    ../include/dimpl/bind.h     bind.cpp    
    #../include/dimpl/emit.h     emit.cpp    
    ../include/dimpl/comp.h     comp.cpp    
    ../include/dimpl/lexer.h    lexer.cpp   
    ../include/dimpl/parser.h   parser.cpp  
)

add_executable(dimpl main.cpp ${BACKEND})

target_include_directories(libdimpl PUBLIC ${Thorin_INCLUDE_DIRS} ${CMAKE_CURRENT_SOURCE_DIR}/../include)
target_link_libraries(libdimpl PUBLIC ${Thorin_LIBRARIES})

target_compile_definitions(dimpl PUBLIC -DDIMPL_VERSION_MAJOR=${PROJECT_VERSION_MAJOR} -DDIMPL_VERSION_MINOR=${PROJECT_VERSION_MINOR})
target_link_libraries(dimpl PUBLIC libdimpl)

if (Thorin_HAS_LLVM_SUPPORT)
    target_compile_definitions(dimpl PUBLIC -DENABLE_LLVM)
    llvm_config(dimpl ${AnyDSL_LLVM_LINK_SHARED} core)
endif ()
