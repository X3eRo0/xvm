#include "breakpoints.h"
#include <commands.h>
#include <iface.h>
#include <stdio.h>
#include <string.h>
#include <xdbg.h>

iface_state state = { 0 };
iface_cmd invalid_cmd = { .cmd = NULL, .method = cmd_invalid };

const iface_cmd cmds[] = {
    { .cmd = "clear", .desc = "Clear screen.", .method = cmd_clear },
    { .cmd = "help", .desc = "Display help messages.", .method = cmd_help },
    { .cmd = "info", .desc = "Display binary header information.", .method = cmd_info },
    { .cmd = "load", .desc = "Load a binary for debugging.", .method = cmd_load },
    { .cmd = "regs", .desc = "Print out all registers and enhance the information.", .method = cmd_regs },
    { .cmd = "disasm", .desc = "Disassemble a symbol.", .method = cmd_disasm },
    { .cmd = "x", .desc = "Examine memory/registers.", .method = cmd_xamine },
    { .cmd = "set", .desc = "Modify memory/registers.", .method = cmd_set },
    { .cmd = "r", .desc = "Run the binary.", .method = cmd_run },
    { .cmd = "continue", .desc = "Continue execution.", .method = cmd_continue },
    { .cmd = "stop", .desc = "Stop the binary.", .method = cmd_stop },
    { .cmd = "vmmap", .desc = "Print section mappings.", .method = cmd_vmmap },
    { .cmd = "tele", .desc = "Recursively dereferences pointers starting at the specified address.", .method = cmd_tele },
    { .cmd = "context", .desc = "Display XVM State.", .method = cmd_context },
    { .cmd = "break", .desc = "Add a breakpoint.", .method = cmd_break },
    { .cmd = "show", .desc = "Show all breakpoints.", .method = cmd_showbp },
    { .cmd = "disable", .desc = "Disable breakpoint.", .method = cmd_disable },
    { .cmd = "enable", .desc = "Enable breakpoint.", .method = cmd_enable },
    { .cmd = "delete", .desc = "Delete breakpoint.", .method = cmd_delete },
    { .cmd = "exit", .desc = "Exit the debugger.", .method = cmd_exit },
    { .cmd = "q", .desc = "Exit the debugger.", .method = cmd_exit },
    { .cmd = NULL, .desc = NULL, .method = NULL }
};

void readcmd(const char* prompt, char* buffer, u32 len)
{
    printf("%s", prompt);
    buffer[read(0, buffer, len) - 1] = 0;
    return;
}

u32 evalcmd(iface_state* state, const char* args[])
{
    // traverse the cmds array to find which command
    // has been envoked and execute the corresponding
    // command method

    for (int i = 0; cmds[i].cmd; i++) {
        if (args[0] == NULL) {
            return E_OK;
        }
        if (!strncmp(args[0], cmds[i].cmd, strlen(cmds[i].cmd))) {
            cmds[i].method(state, args);
            return E_OK;
        }
    }
    invalid_cmd.method(state, args);
    return E_ERR;
}

void xdbg_iface(iface_state* state, const char* name)
{
    // iface state
    char command[IFACE_MAX_CMD_SZ] = { 0 };
    char* args[IFACE_MAX_CMD_ARGS] = { 0 };

    state->rflag = true;
    state->bps = NULL;
    load_binary(state, name);

    set_RF(state->cpu, 0);

    while (state->rflag) {
        // read and evaluate commands
        if (get_RF(state->cpu)) {
            dbg_cpu(state);
        } else {
            repl_state* rs = repl_init("xdbg> ");
            repl_readline(rs, command, IFACE_MAX_CMD_SZ);
            xdbg_parse_args(command, args);
            u32 ret = evalcmd(state, (const char**)args);
            if (ret == E_ERR) {
                // do something
                ;
            }
        }
    }

    unload_binary(state);
}
