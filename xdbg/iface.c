#include <commands.h>
#include <iface.h>
#include <stdio.h>
#include <string.h>
#include <xdbg.h>

iface_cmd invalid_cmd = { .cmd = NULL, .method = cmd_invalid };

const iface_cmd cmds[] = {
    { .cmd = "help", .desc = "Display help messages.", .method = cmd_help },
    { .cmd = "regs", .desc = "Print out all registers and enhance the information.", .method = cmd_regs },
    { .cmd = "disasm", .desc = "Disassemble a symbol.", .method = cmd_disasm },
    { .cmd = "exit", .desc = "Exit the debugger.", .method = cmd_exit },
    { .cmd = NULL, .desc = NULL, .method = NULL }
};

void readcmd(const char* prompt, char* buffer, u32 len)
{
    printf("%s", prompt);
    buffer[read(0, buffer, len) - 1] = 0;
    return;
}

u32 evalcmd(iface_state* state, const char* command)
{
    // traverse the cmds array to find which command
    // has been envoked and execute the corresponding
    // command method

    for (int i = 0; cmds[i].cmd; i++) {
        if (command[i] == '\0') {
            return E_OK;
        }
        if (!strncmp(command, cmds[i].cmd, strlen(cmds[i].cmd))) {
            cmds[i].method(state, command);
            return E_OK;
        }
    }
    invalid_cmd.method(state, command);
    return E_ERR;
}

void xdbg_iface(xvm_cpu* cpu, xvm_bin* bin)
{
    // iface state
    iface_state state = { 0 };
    char command[IFACE_MAX_CMD_SZ] = { 0 };

    state.rflag = true;
    state.cpu = cpu;
    state.bin = bin;
    while (state.rflag) {
        // read and evaluate commands
        readcmd("xdbg> ", command, IFACE_MAX_CMD_SZ);
        char* tempcmd = (char*)command;
        clear_whitespaces(tempcmd);
        u32 ret = evalcmd(&state, tempcmd);
        if (ret == E_ERR) {
            // do something
            ;
        }
    }
}
