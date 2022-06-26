#ifndef COMMANDS_H
#define COMMANDS_H
#include <const.h>
#include <iface.h>
#include <const.h>
#include <sections.h>
#include <symbols.h>
#include <xasm.h>
#include <xdbg.h>
#include <stdarg.h>

typedef enum {
    TYPE_BYTES = 0,
    TYPE_WORDS,
    TYPE_DWORD,
    TYPE_STRING,
    TYPE_DISASM,
} xdbg_dump_types;

// string processing Macros
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

// helper functions
void xdbg_parse_args(char* cmd, char* args[]);
void xdbg_info(char* fmt, ...);
void xdbg_error(char* fmt, ...);
void xdbg_hexdump(FILE* fp, const char* buffer, u32 size, u32 type, u32 address);
void xdbg_print_register(iface_state* state, xvm_registers regid);

u32 cmd_clear(iface_state* state, const char* args[]);
u32 cmd_help(iface_state* state, const char* args[]);
u32 cmd_info(iface_state* state, const char* args[]);
u32 cmd_load(iface_state* state, const char* args[]);
u32 cmd_regs(iface_state* state, const char* args[]);
u32 cmd_disasm(iface_state* state, const char* args[]);
u32 cmd_xamine(iface_state* state, const char* args[]);
u32 cmd_set(iface_state* state, const char* args[]);
u32 cmd_tele(iface_state* state, const char* args[]);
u32 cmd_vmmap(iface_state* state, const char* args[]);
u32 cmd_run(iface_state* state, const char* args[]);
u32 cmd_continue(iface_state* state, const char* args[]);
u32 cmd_stop(iface_state* state, const char* args[]);
u32 cmd_context(iface_state* state, const char* args[]);
u32 cmd_break(iface_state* state, const char* args[]);
u32 cmd_showbp(iface_state* state, const char* args[]);
u32 cmd_disable(iface_state* state, const char* args[]);
u32 cmd_enable(iface_state* state, const char* args[]);
u32 cmd_delete(iface_state* state, const char* args[]);
u32 cmd_exit(iface_state* state, const char* args[]);

// invalid command
u32 cmd_invalid(iface_state* state, const char* args[]);

#endif // !COMMANDS_H
