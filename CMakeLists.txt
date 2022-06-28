cmake_minimum_required(VERSION 3.16)
project(xvm C)

set(CMAKE_C_STANDARD 11)
set(CMAKE_EXPORT_COMPILE_COMMANDS ON)
# set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} -fsanitize=address -g -O0")
set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} -g -O0")

add_executable(xvm
    xvm/xvm.c
    xvm/cpu.c
    xvm/cpu.h
    xvm/execute.c
    xvm/syscall.c
    common/signals.c
    common/signals.h
    common/symbols.c
    common/symbols.h
    common/const.h
    common/loader.c
    common/loader.h
    common/sections.c
    common/sections.h
)
target_include_directories(xvm PUBLIC xvm common)

add_executable(xdbg
    xdbg/xdbg.c
    xdbg/debug.c
    xdbg/iface.c
    xdbg/commands.c
    xdbg/helperfuncs.c
    xdbg/breakpoints.c
    xdbg/repl.c
    xvm/cpu.c
    xvm/cpu.h
    xvm/execute.c
    xvm/syscall.c
    xasm/disasm.c
    xasm/mnemonics.c
    xasm/xasm_functions.c
    common/cstring.c
    common/signals.c
    common/signals.h
    common/symbols.c
    common/symbols.h
    common/const.h
    common/loader.c
    common/loader.h
    common/sections.c
    common/sections.h
)
target_include_directories(xdbg PUBLIC xdbg xvm common xasm)

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
target_include_directories(xasm PUBLIC xasm common)

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
target_include_directories(xinfo PUBLIC xasm common)

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
target_include_directories(xdis PUBLIC xasm common)

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

target_include_directories(ropgadget PUBLIC ropgadget xasm common)
