add_library(libdimpl
    ../include/dimpl/ast.h      stream.cpp
    ../include/dimpl/bind.h     bind.cpp    
    #../include/dimpl/emit.h     emit.cpp    
    ../include/dimpl/comp.h     comp.cpp    
    ../include/dimpl/lexer.h    lexer.cpp   
    ../include/dimpl/parser.h   parser.cpp  
)

add_dependencies(libdimpl thorin)
target_include_directories(libdimpl PUBLIC ${Thorin_INCLUDE_DIRS} ${CMAKE_CURRENT_SOURCE_DIR}/../include)
target_link_libraries(libdimpl PUBLIC ${Thorin_LIBRARIES})

