#include "iface.h"
#include <commands.h>

u32 cmd_help(iface_state* state, const char* command)
{
    printf("[+] help command\n");
    return E_OK;
}

u32 cmd_regs(iface_state* state, const char* command)
{
    xvm_cpu* cpu = state->cpu;
    printf("PC -- 0x%.8X [ZF : %s] [CF : %s]\n", cpu->regs.pc, (get_ZF(cpu) == 1 ? "True" : "False"), (get_CF(cpu) == 1 ? "True" : "False"));
    printf("$r0 : 0x%.8X\t$r1 : 0x%.8X\t$r2 : 0x%.8X\t$r3 : 0x%.8X\n", cpu->regs.r0, cpu->regs.r1, cpu->regs.r2, cpu->regs.r3);
    printf("$r4 : 0x%.8X\t$r5 : 0x%.8X\t$r6 : 0x%.8X\t$r7 : 0x%.8X\n", cpu->regs.r4, cpu->regs.r5, cpu->regs.r6, cpu->regs.r7);
    printf("$r8 : 0x%.8X\t$r9 : 0x%.8X\t$ra : 0x%.8X\t$rb : 0x%.8X\n", cpu->regs.r8, cpu->regs.r9, cpu->regs.ra, cpu->regs.rb);
    printf("$rc : 0x%.8X\t$pc : 0x%.8X\t$bp : 0x%.8X\t$sp : 0x%.8X\n", cpu->regs.rc, cpu->regs.pc, cpu->regs.bp, cpu->regs.sp);
    return E_OK;
}

u32 cmd_exit(iface_state* state, const char* command)
{
    printf("[+] exit command\n");
    return E_OK;
}

u32 cmd_invalid(iface_state* state, const char* command)
{
    printf("[+] invalid command\n");
    return E_OK;
}
