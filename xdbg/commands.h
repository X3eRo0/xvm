#ifndef COMMANDS_H
#define COMMANDS_H
#include <const.h>
#include <iface.h>
#include <const.h>
#include <sections.h>
#include <symbols.h>
#include <xasm.h>
#include <stdarg.h>

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
void xdbg_info(char* fmt, ...);
void xdbg_error(char* fmt, ...);
void print_register(iface_state* state, xvm_registers regid);

u32 cmd_help(iface_state* state, const char* command);
u32 cmd_regs(iface_state* state, const char* command);
u32 cmd_disasm(iface_state* state, const char* command);
u32 cmd_exit(iface_state* state, const char* command);

// invalid command
u32 cmd_invalid(iface_state* state, const char* command);

#endif // !COMMANDS_H
