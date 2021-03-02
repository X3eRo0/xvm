cmake_minimum_required(VERSION 3.16)
project(xvm C)

set(CMAKE_C_STANDARD 11)

add_executable(xvm
    xvm/xvm.c
    xvm/memory.c
    xvm/memory.h
    common/symbols.c
    common/symbols.h
    common/const.h
    common/loader.c
    common/loader.h
    common/sections.c
    common/sections.h
)

add_executable(xasm
    xasm/xasm.c
    xasm/xasm.h
    xasm/xasm_functions.c
    xasm/parse.c
    common/symbols.c
    common/symbols.h
    common/sections.c
    common/sections.h
    common/loader.c
    common/loader.h
)