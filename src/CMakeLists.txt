add_library(libdimpl
    #emit.cpp    
    bind.cpp    
    comp.cpp    
    lexer.cpp   
    parser.cpp  
    stream.cpp
)

add_definitions(-fno-rtti)
add_dependencies(libdimpl thorin)
target_include_directories(libdimpl PUBLIC ${Thorin_INCLUDE_DIRS} ${CMAKE_CURRENT_SOURCE_DIR}/../include)
target_link_libraries(libdimpl PUBLIC ${Thorin_LIBRARIES})
