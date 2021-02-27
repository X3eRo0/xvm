//
// Created by X3eRo0 on 2/23/2021.
//

#ifndef XVM_XASM_H
#define XVM_XASM_H

#include <stdio.h>
#include <stdbool.h>
#include <stdarg.h>
#include <ctype.h>
#include "../common/symbols.h"

#define XVM_NINSTR  5
#define XVM_NREGS   7


// enum for opcodes

typedef enum {

    opc_mov,
    opc_hlt,
    opc_ret,
    opc_call,
    opc_syscall,

} xvm_opcodes;


// enum for xvm registers

typedef enum {

    reg_r0,
    reg_r1,
    reg_r2,
    reg_r3,
    reg_r4,
    reg_pc,
    reg_sp,

} xvm_registers;


// enum for types of
// arguments to instruction

typedef enum {

    arg_noarg = 0,      // ret
    arg_register = 1,   // xor $r0, $r1
    arg_immediate = 2,  // mov $r0, #0x41414141
    arg_offset = 4,     // call #offset, jump #offset
    arg_pointer = 8,    // mov [$r0] $r1; mov [#0x1234], $r1; mov [$r0+#0x1234], $r1

} xasm_argument_t;

typedef enum {

    E_INVALID_IMMEDIATE,
    E_INVALID_SYNTAX,
    E_INVALID_OPCODE,

} xasm_error_id;


#define is_line_empty(line) \
                    (line)[0] == '\n' || \
                    (line)[0] == '\x00' || \
                    (line)[0] == '\t' || \
                    strlen(line) == 0

#define is_white_space(line) \
                    ((line)[0] == ' ') || \
                    ((line)[0] == '\t') || \
                    ((line)[0] == '\b')

#define clear_whitespaces(line) \
                    while ((line)[0] != '\x00'){ \
                        if (is_white_space(line)){ \
                            (line)++; \
                        } else { \
                            break; \
                        } \
                    }

#define is_digit(ch)  ((ch) >= '0') && ((ch) <= '9')
#define is_hex(ch)  (is_digit(ch) || ((ch) >= 'a' && (ch) <= 'f') || ((ch) >= 'A' && (ch) <= 'F'))
#define is_binary(ch)  ((ch) == '0' || (ch) == '1')

typedef struct xasm_t {

    FILE*   ifile;  // input file
    FILE*   ofile;  // output file
    symtab* symtab; // symbol table

} xasm;

typedef struct arg_t {

    u32 arg_type;   // register, immediate, pointer
    u32 opt_value;  // if immediate
    u32 opt_regid;  // if register
    u32 opt_offset; // if pointer


} arg;


xasm*   init_xasm();
arg*    init_arg();
u32     disp_arg(arg* x_arg);
u32     fini_arg(arg* x_arg);
u32     get_sign(char s_sign);
u32     xasm_escape_string(char* unescaped);
u32     xasm_resolve_number(char* num_s);
char*   xasm_resolve_register(u32 reg_id);
u32     xasm_resolve_register_id(char* reg_s);
u32     xasm_resolve_opcode(char* mnemonic);
u32     xasm_resolve_argument(arg* arg, symtab* symtab, char* args, bool calc_size);
char*   xasm_resolve_mnemonic(u32 opcode);
u32     xasm_assemble(xasm* xasm);
u32     xasm_assemble_line(xasm* xasm, char* line, bool calc_size);
u32     open_ifile(xasm* xasm, char* file);
u32     open_ofile(xasm* xasm, char* file);
u32     close_ifile(xasm* xasm);
u32     close_ofile(xasm* xasm);
u32     fini_xasm(xasm* xasm);
char*   strchrnul(const char*, int c);
u32     xasm_error(u32 error_id, u32 line, char* func, char* msg, ...);
#endif //XVM_XASM_H
