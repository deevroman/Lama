cmake_minimum_required(VERSION 3.10)
project(byterun C)

include_directories(${CMAKE_CURRENT_SOURCE_DIR}/../interp)

add_executable(byterun
        byterun.c
        ../interp/disasm_format.h
        ../runtime/runtime_common.h
        ../runtime/runtime.c
        ../runtime/runtime.h
        ../runtime/gc.c
        ../runtime/gc.h
)
#
#target_compile_options(byterun PRIVATE
#        -g
#        -fsanitize=address
#        -fsanitize=undefined
#)
#
#target_link_options(byterun PRIVATE
#        -fsanitize=address
#        -fsanitize=undefined
#)

add_executable(new_byterun
        new_byterun.c
        ../runtime/runtime_common.h
        ../runtime/runtime.c
        ../runtime/runtime.h
        ../runtime/gc.c
        ../runtime/gc.h
)

#target_link_libraries(byterun PRIVATE ${CMAKE_CURRENT_SOURCE_DIR}/../runtime/runtime.a)
