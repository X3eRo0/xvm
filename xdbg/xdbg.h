#ifndef XDBG_H
#define XDBG_H

#include <cpu.h>
#include <iface.h>

void unload_binary(iface_state* state);
void load_binary(iface_state* state, const char* filename);
void handle_signals(iface_state* state);
void dbg_cpu(iface_state* state);

#endif // !XDBG_H
