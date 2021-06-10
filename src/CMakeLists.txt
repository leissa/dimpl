add_library(libdimpl
    bind.cpp    
    comp.cpp    
    lexer.cpp   
    parser.cpp  
    stream.cpp
)

target_compile_options    (libdimpl PUBLIC -fno-rtti PRIVATE -Wall -Wextra)
target_include_directories(libdimpl PUBLIC ${PROJECT_SOURCE_DIR}/include)
target_link_libraries     (libdimpl PUBLIC thorin)
