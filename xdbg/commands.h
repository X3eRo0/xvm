#ifndef COMMANDS_H
#define COMMANDS_H
#include <iface.h>


u32 cmd_help(iface_state* state, const char* command);
u32 cmd_regs(iface_state* state, const char* command);
u32 cmd_exit(iface_state* state, const char* command);

// invalid command
u32 cmd_invalid(iface_state* state, const char* command);

#endif // !COMMANDS_H
