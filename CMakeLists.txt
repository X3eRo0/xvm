cmake_minimum_required(VERSION 3.16)
project(xvm C)

set(CMAKE_C_STANDARD 11)

add_executable(xvm xvm.c symbols.c symbols.h memory.c memory.h const.h loader.c loader.h)
add_executable(xasm xasm.c xasm.h symbols.c symbols.h xasm_functions.c)