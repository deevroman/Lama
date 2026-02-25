cmake_minimum_required(VERSION 3.10)
project(byterun C)

add_executable(byterun byterun.c)
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

target_link_libraries(byterun PRIVATE ${CMAKE_CURRENT_SOURCE_DIR}/../runtime/runtime.a)
