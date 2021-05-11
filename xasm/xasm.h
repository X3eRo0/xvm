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
#include "../common/sections.h"
#include "../common/loader.h"


#define XVM_NINSTR  48
#define XVM_NREGS   16


// enum for opcodes

typedef enum {
    XVM_OP_MOV,
    XVM_OP_MOVB,
    XVM_OP_MOVW,
    XVM_OP_NOP,
    XVM_OP_HLT,
    XVM_OP_RET,
    XVM_OP_CALL,
    XVM_OP_SYSC,
    XVM_OP_ADD,
    XVM_OP_ADDB,
    XVM_OP_ADDW,
    XVM_OP_SUB,
    XVM_OP_SUBB,
    XVM_OP_SUBW,
    XVM_OP_MUL,
    XVM_OP_MULB,
    XVM_OP_MULW,
    XVM_OP_DIV,
    XVM_OP_DIVB,
    XVM_OP_DIVW,
    XVM_OP_XOR,
    XVM_OP_XORB,
    XVM_OP_XORW,
    XVM_OP_AND,
    XVM_OP_ANDB,
    XVM_OP_ANDW,
    XVM_OP_OR,
    XVM_OP_ORB,
    XVM_OP_ORW,
    XVM_OP_NOT,
    XVM_OP_NOTB,
    XVM_OP_NOTW,
    XVM_OP_PUSH,
    XVM_OP_POP,
    XVM_OP_XCHG,
    XVM_OP_INC,
    XVM_OP_DEC,
    XVM_OP_CMP,
    XVM_OP_CMPB,
    XVM_OP_CMPW,
    XVM_OP_TEST,
    XVM_OP_JMP,
    XVM_OP_JZ,
    XVM_OP_JNZ,
    XVM_OP_JA,
    XVM_OP_JB,
    XVM_OP_JAE,
    XVM_OP_JBE,
} xvm_opcodes;


// enum for xvm registers

typedef enum {

    reg_r0,
    reg_r1,
    reg_r2,
    reg_r3,
    reg_r4,
    reg_r5,
    reg_r6,
    reg_r7,
    reg_r8,
    reg_r9,
    reg_ra,
    reg_rb,
    reg_rc,
    reg_pc,
    reg_bp,
    reg_sp,

} xvm_registers;


// enum for types of
// arguments to instruction

typedef enum {

    ARG_NARG = 0,  // ret
    ARG_REGD = 1,  // xor $r0, $r1
    ARG_IMMD = 2,  // mov $r0, #0x41414141
    ARG_PTRD = 4,  // mov $r1, [$r0]; mov $r1, [#0x1234]; mov $r1, [$r0+#0x1234]

} xasm_argument_t;

typedef enum {

    E_INVALID_IMMEDIATE,
    E_INVALID_SYNTAX,
    E_INVALID_OPCODE,
    E_NOFILE,

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

#define skip_to_whitespace(line) \
                    while ((line)[0] != '\x00'){ \
                        if (!is_white_space(line)){\
                             (line)++;\
                        } else { \
                            break; \
                        } \
                    }


#define is_digit(ch)  ('0' <= (ch) && (ch) <= '9')
#define is_hex(ch)  (is_digit(ch) || ((ch) >= 'a' && (ch) <= 'f') || ((ch) >= 'A' && (ch) <= 'F'))
#define is_binary(ch)  ((ch) == '0' || (ch) == '1')

typedef struct xasm_t {

    FILE*       ifile;     // input file
    FILE*       ofile;     // output file
    symtab*     symtab;    // symbol table
    symtab*     define;    // symbol table for #define directive
    section*    sections;  // sections
    exe_header* header;
} xasm;

typedef struct arg_t {

    u32 arg_type;   // register, immediate, pointer
    u32 opt_value;  // if immediate or pointer
    u32 opt_regid;  // if register
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
u32     xasm_resolve_argument(arg* arg, xasm* xasm, char* args, bool calc_size);
char*   xasm_resolve_mnemonic(u32 opcode);
void    xasm_disassemble(xvm_bin* bin, u32 ninstr);
u32     xasm_assemble(xasm *xasm, section_entry *default_section_entry, FILE **inputf, u32 ifiles);
u32     xasm_assemble_line(xasm* xasm, char* line, section_entry** current_section_entry, bool calc_size);
u32     xasm_open_ifile(xasm* xasm, char* file);
u32     xasm_open_ofile(xasm* xasm, char* file);
u32     xasm_close_ifile(xasm* xasm);
u32     xasm_close_ofile(xasm* xasm);
u32     fini_xasm(xasm* xasm);
u32     get_total_size(xasm* xasm);
char*   strchrnul(const char*, int c);
u32     xasm_error(u32 error_id, u32 line, char* func, char* msg, ...);
#endif //XVM_XASM_H
