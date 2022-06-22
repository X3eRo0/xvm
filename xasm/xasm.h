//
// Created by X3eRo0 on 2/23/2021.
//

#ifndef XVM_XASM_H
#define XVM_XASM_H

#include <stdio.h>
#include <stdbool.h>
#include <stdarg.h>
#include <ctype.h>
#include <symbols.h>
#include <sections.h>
#include <loader.h>


#define XVM_NREGS   16

// enum for xvm registers

extern const char *mnemonics[XVM_OP_LAST];
extern const int inst_to_args_dict[XVM_OP_LAST];
extern const char* regid_2_str[XVM_NREGS];

// enum for types of
// arguments to instruction

typedef enum {

    ARG_NARG = 0,  // ret
    ARG_REGD = 1,  // xor $r0, $r1
    ARG_IMMD = 2,  // mov $r0, #0x41414141
    ARG_PTRD = 4,  // mov $r1, [$r0]; mov $r1, [#0x1234]; mov $r1, [$r0+#0x1234]

} xasm_argument_t;

typedef enum{
    ARG0,
    ARG1,
    ARG2,
} xasm_no_of_args;

typedef enum {
    E_INVALID_INPUTFILE,
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
u32     resolve_number(char* num_s, u32* number);
u32     xasm_resolve_number(char* num_s);
char*   xasm_resolve_register(u32 reg_id);
u32     xasm_resolve_register_id(char* reg_s);
u32     xasm_resolve_opcode(char* mnemonic);
u32     xasm_resolve_argument(arg* arg, xasm* xasm, char* args, bool calc_size);
char*   xasm_resolve_mnemonic(u32 opcode);
u32     xasm_disassemble_bytes_uncolored(FILE* fp, xvm_bin* bin, const char* bytecode, u32 len, u32 address, u32 ninstr, u32 need_clean);
u32     xasm_disassemble_bytes_colored(FILE* fp, xvm_bin* bin, const char* bytecode, u32 len, u32 address, u32 ninstr, u32 need_clean);
u32     xasm_disassemble_bytes(FILE* fp, xvm_bin* bin, const char* bytecode, u32 len, u32 address, u32 ninstr, u32 need_clean);
u32     internal_xasm_disassemble_bytes(FILE* fp, xvm_bin* bin, const char* bytecode, u32 len, u32 address, u32 ninstr, u32 need_clean, u32 colored);
u32     xasm_assemble(xasm *xasm, section_entry *default_section_entry, FILE **inputf, u32 ifiles);
u32     xasm_assemble_line(xasm* xasm, char* line, section_entry** current_section_entry, bool calc_size);
u32     xasm_open_ifile(xasm* xasm, char* file);
u32     xasm_open_ofile(xasm* xasm, char* file);
u32     xasm_close_ifile(xasm* xasm);
u32     xasm_close_ofile(xasm* xasm);
u32     fini_xasm(xasm* xasm);
u32     get_total_size(xasm* xasm);
char*   strchrnul(const char*, int c);
void    xasm_info(char* msg, ...);
void    xasm_warn(char* msg, ...);
void    xasm_error(u32 error_id, u32 line, char* func, char* msg, ...);
#endif //XVM_XASM_H
