cmake_minimum_required(VERSION 3.16)
project(xvm C)

set(CMAKE_C_STANDARD 11)

add_executable(xvm xvm/xvm.c common/symbols.c common/symbols.h xvm/memory.c xvm/memory.h common/const.h common/loader.c common/loader.h)
add_executable(xasm xasm/xasm.c xasm/xasm.h common/symbols.c common/symbols.h xasm/xasm_functions.c xasm/parse.c)