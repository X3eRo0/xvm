#ifndef IFACE_H
#define IFACE_H

#include <breakpoints.h>
#include <cpu.h>
#define IFACE_MAX_CMD_SZ 200
#define IFACE_MAX_CMD_ARGS 10

typedef struct {
    u8 rflag; // running flag
    const char* name; // binary name
    xvm_cpu* cpu; // cpu struct
    xvm_bin* bin; // bin struct
    breaklist* bps; // breakpoint list
} iface_state;

typedef struct
{
    const char* cmd;
    const char* desc;
    u32 (*method)(iface_state* state, const char* args[]);
} iface_cmd;

extern const iface_cmd cmds[];

void readcmd(const char* prompt, char* buffer, u32 len);
u32 evalcmd(iface_state* state, const char* args[]);
void xdbg_iface(iface_state* state, const char * name);
#endif // !IFACE_H
