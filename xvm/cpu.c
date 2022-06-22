//
// Created by X3eRo0 on 4/15/2021.
//
#include <cpu.h>

void reset_reg(xvm_reg* regs)
{
    memset(regs, 0, sizeof(xvm_reg));
    regs->pc = XVM_DFLT_EP;
    regs->sp = XVM_DFLT_SP;
    regs->bp = XVM_DFLT_SP;
}

void reset_flags(xvm_flags* flags)
{
    flags->flags = (1 << XVM_RF);
}

u8 get_RF(xvm_cpu* cpu)
{
    return cpu->flags.flags & (1 << XVM_RF);
}

u8 get_ZF(xvm_cpu* cpu)
{
    return cpu->flags.flags & (1 << XVM_ZF);
}

u8 get_CF(xvm_cpu* cpu)
{
    return cpu->flags.flags & (1 << XVM_CF);
}

u8 get_SF(xvm_cpu* cpu)
{
    return cpu->flags.flags & (1 << XVM_SF);
}

u8 set_SF(xvm_cpu* cpu, u8 bit)
{
    if (bit) {
        cpu->flags.flags |= (1 << XVM_SF);
    } else {
        cpu->flags.flags &= ~(1 << XVM_SF);
    }
    return cpu->flags.flags & (1 << XVM_SF);
}

u8 set_RF(xvm_cpu* cpu, u8 bit)
{
    if (bit) {
        cpu->flags.flags |= (1 << XVM_RF);
    } else {
        cpu->flags.flags &= ~(1 << XVM_RF);
    }
    return cpu->flags.flags & (1 << XVM_RF);
}

u8 set_ZF(xvm_cpu* cpu, u8 bit)
{
    if (bit) {
        cpu->flags.flags |= (1 << XVM_ZF);
    } else {
        cpu->flags.flags &= ~(1 << XVM_ZF);
    }
    return cpu->flags.flags & (1 << XVM_ZF);
}

u8 set_CF(xvm_cpu* cpu, u8 bit)
{
    if (bit) {
        cpu->flags.flags |= (1 << XVM_CF);
    } else {
        cpu->flags.flags &= ~(1 << XVM_CF);
    }
    return cpu->flags.flags & (1 << XVM_CF);
}

xvm_cpu* init_xvm_cpu()
{
    xvm_cpu* cpu = (xvm_cpu*)malloc(sizeof(xvm_cpu));
    cpu->errors = (signal_report*)calloc(1, sizeof(signal_report));
    reset_reg(&cpu->regs);
    reset_flags(&cpu->flags);

    return cpu;
}

// FIXME: REMOVE IN RELEASE

/*
void show_registers(xvm_cpu* cpu, xvm_bin * bin){
    printf("\n\nPC -- 0x%.8X [ZF : %s] [CF : %s]\n", cpu->regs.pc, (get_ZF(cpu)==1? "True" : "False"), (get_CF(cpu)==1? "True" : "False"));
    printf("$r0 : 0x%.8X\n$r1 : 0x%.8X\n$r2 : 0x%.8X\n$r3 : 0x%.8X\n", cpu->regs.r0, cpu->regs.r1, cpu->regs.r2, cpu->regs.r3);
    printf("$r4 : 0x%.8X\n$r5 : 0x%.8X\n$r6 : 0x%.8X\n$r7 : 0x%.8X\n", cpu->regs.r4, cpu->regs.r5, cpu->regs.r6, cpu->regs.r7);
    printf("$r8 : 0x%.8X\n$r9 : 0x%.8X\n$ra : 0x%.8X\n$rb : 0x%.8X\n", cpu->regs.r8, cpu->regs.r9, cpu->regs.ra, cpu->regs.rb);
    printf("$rc : 0x%.8X\n$pc : 0x%.8X\n$bp : 0x%.8X\n$sp : 0x%.8X\n", cpu->regs.rc, cpu->regs.pc, cpu->regs.bp, cpu->regs.sp);
    //for (u32 i = XVM_DFLT_SP; i >= cpu->regs.sp; i -= 4){
    //    printf("0x%.8X : 0x%.8X\n", i, read_dword(bin->x_section, i, PERM_READ));
    //}
}
*/

u32 signal_abort(signal_report* err, xvm_cpu* cpu)
{
    if (err != NULL) {

        if (err->signal_id == NOSIGNAL) {
            return E_OK;
        }

        set_RF(cpu, 0);

        switch (err->signal_id) {
        case XSIGFPE:
            fprintf(stderr /*fp*/, "[-] Floating point exception (core not dumped)\n");
            break;
        case XSIGILL:
            fprintf(stderr /*fp*/, "[-] Illegal Instruction (core not dumped)\n");
            break;
        case XSIGTRAP:
            fprintf(stderr /*fp*/, "[-] Trap/Breakpoint\n");
            break;
        case XSIGSTOP:
            break;
        default:
            fprintf(stderr /*fp*/, "[-] Segmentation Fault (core not dumped)\n");
            break;
        }
    }
    return E_ERR;
}

void fde_cpu(xvm_cpu* cpu, xvm_bin* bin)
{
    u32 instr_size = 0;
    while (get_RF(cpu)) {
        // show_registers(cpu, bin);
        instr_size = do_execute(cpu, bin);
        if (signal_abort(cpu->errors, cpu) == E_ERR) {
            return;
        }
        if (signal_abort(bin->x_section->errors, cpu) == E_ERR) {
            return;
        }
    }
}

void fini_xvm_cpu(xvm_cpu* cpu)
{
    free(cpu->errors);
    memset(cpu, 0, sizeof(xvm_cpu));
    free(cpu);
    cpu = NULL;
}
