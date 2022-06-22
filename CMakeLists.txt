cmake_minimum_required(VERSION 3.16)
project(xvm C)

set(CMAKE_C_STANDARD 11)
# set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} -fsanitize=address -g -O0")
set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} -g -O0")

add_executable(xvm
    xvm/xvm.c
    xvm/cpu.c
    xvm/cpu.h
    xvm/execute.c
    common/signals.c
    common/signals.h
    common/symbols.c
    common/symbols.h
    common/const.h
    common/loader.c
    common/loader.h
    common/sections.c
    common/sections.h
    xvm/syscall.c
)

add_executable(xasm
    xasm/xasm.c
    xasm/xasm.h
    xasm/xasm_functions.c
    xasm/parse.c
    xasm/mnemonics.c
    common/signals.c
    common/signals.h
    common/symbols.c
    common/symbols.h
    common/sections.c
    common/sections.h
    common/loader.c
    common/loader.h
)

add_executable(xinfo
    xasm/xinfo.c
    common/loader.c
    common/loader.h
    common/symbols.c
    common/symbols.h
    common/signals.c
    common/signals.h
    common/sections.c
    common/sections.h
)

add_executable(xdis
    xasm/xdis.c
    xasm/disasm.c
    xasm/xasm_functions.c
    xasm/mnemonics.c
    common/loader.c
    common/loader.h
    common/symbols.c
    common/symbols.h
    common/signals.c
    common/signals.h
    common/sections.c
    common/sections.h
)

add_executable(ropgadget
    ropgadget/ropgadget.c
    ropgadget/ropgadget.h
    ropgadget/findgadgets.c
    xasm/xasm.h
    xasm/xasm_functions.c
    xasm/disasm.c
    xasm/mnemonics.c
    common/loader.c
    common/loader.h
    common/symbols.c
    common/symbols.h
    common/signals.c
    common/signals.h
    common/sections.c
    common/sections.h
)
