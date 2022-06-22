#ifndef IFACE_H
#define IFACE_H

#include <cpu.h>
#define IFACE_MAX_CMD_SZ 200

typedef struct {
    u8 rflag; // running flag
    char* name; // binary name
    xvm_cpu* cpu; // cpu struct
    xvm_bin* bin; // bin struct
} iface_state;

typedef struct
{
    const char* cmd;
    const char* desc;
    u32 (*method)(iface_state* state, const char* command);
} iface_cmd;

extern const iface_cmd cmds[];

void readcmd(const char* prompt, char* buffer, u32 len);
u32 evalcmd(iface_state* state, const char* command);
void xdbg_iface(xvm_cpu* cpu, xvm_bin* bin);
#endif // !IFACE_H
