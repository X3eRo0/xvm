#ifndef XDBG_H
#define XDBG_H

#include <cpu.h>
#include <iface.h>

void handle_signals(xvm_cpu* cpu, xvm_bin* bin);
void dbg_cpu(xvm_cpu* cpu, xvm_bin* bin);

#endif // !XDBG_H
