cmake_minimum_required(VERSION 3.10)
project(lama_interpreter C)

set(CMAKE_C_STANDARD 23)

add_compile_options(-O2)
#add_compile_options(-O3)
add_compile_options(-g)
#add_compile_options(-fomit-frame-pointer)
#add_compile_options(-fno-strict-aliasing)
add_compile_options(-Wall)
add_compile_options(-Wextra)
#add_compile_options(-DDEBUG)

include_directories(${CMAKE_CURRENT_SOURCE_DIR}/../runtime)

set(SOURCES
    main.c
    loader.h
    interpreter.h
    external.h
    opcodes.h
    callstack.h
    globals.h
    ../runtime/runtime_common.h
    ../runtime/runtime.c
    ../runtime/runtime.h
    ../runtime/gc.c
    ../runtime/gc.h
)

add_executable(lama_interp ${SOURCES})

#target_compile_options(lama_interp PRIVATE
#        -g
#        -fsanitize=address
#        -fsanitize=undefined
#)
#
#target_link_options(lama_interp PRIVATE
#        -fsanitize=address
#        -fsanitize=undefined
#)

#target_link_libraries(lama_interp ${CMAKE_CURRENT_SOURCE_DIR}/../runtime/runtime.a)


