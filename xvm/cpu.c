//
// Created by X3eRo0 on 4/15/2021.
//

#include "cpu.h"

void reset_reg(xvm_reg * regs){
    memset(regs, 0, sizeof(xvm_reg));
    regs->pc = XVM_DFLT_EP;
    regs->sp = XVM_DFLT_SP;
    regs->bp = XVM_DFLT_SP;
}

void reset_flags(xvm_flags * flags){
    flags->flags = (1 << XVM_RF);
}

u8 get_RF(xvm_cpu * cpu){
    return cpu->flags.flags & (1 << XVM_RF);
}

u8 get_ZF(xvm_cpu * cpu){
    return cpu->flags.flags & (1 << XVM_ZF);
}

u8 get_CF(xvm_cpu * cpu){
    return cpu->flags.flags & (1 << XVM_CF);
}

u8 set_RF(xvm_cpu * cpu, u8 bit){
    if (bit){
        cpu->flags.flags |= (1 << XVM_RF);
    } else {
        cpu->flags.flags &= ~(1 << XVM_RF);
    }
    return cpu->flags.flags & (1 << XVM_RF);
}

u8 set_ZF(xvm_cpu * cpu, u8 bit){
    if (bit){
        cpu->flags.flags |= (1 << XVM_ZF);
    } else {
        cpu->flags.flags &= ~(1 << XVM_ZF);
    }
    return cpu->flags.flags & (1 << XVM_ZF);
}

u8 set_CF(xvm_cpu * cpu, u8 bit){
    if (bit){
        cpu->flags.flags |= (1 << XVM_CF);
    } else {
        cpu->flags.flags &= ~(1 << XVM_CF);
    }
    return cpu->flags.flags & (1 << XVM_CF);
}

xvm_cpu * init_xvm_cpu(){
    xvm_cpu * cpu = (xvm_cpu *)malloc(sizeof(xvm_cpu));
    reset_reg(&cpu->regs);
    reset_flags(&cpu->flags);

    return cpu;
}

// FIXME: REMOVE IN RELEASE

//void show_registers(xvm_cpu* cpu){
//    printf("\n\nPC -- 0x%.8X\n", cpu->regs.pc);
//    printf("$r0 : 0x%.8X\t$r1 : 0x%.8X\t$r2 : 0x%.8X\t$r3 : 0x%.8X\n", cpu->regs.r0, cpu->regs.r1, cpu->regs.r2, cpu->regs.r3);
//    printf("$r4 : 0x%.8X\t$r5 : 0x%.8X\t$r6 : 0x%.8X\t$r7 : 0x%.8X\n", cpu->regs.r4, cpu->regs.r5, cpu->regs.r6, cpu->regs.r7);
//    printf("$r8 : 0x%.8X\t$r9 : 0x%.8X\t$ra : 0x%.8X\t$rb : 0x%.8X\n", cpu->regs.r8, cpu->regs.r9, cpu->regs.ra, cpu->regs.rb);
//    printf("$rc : 0x%.8X\t$pc : 0x%.8X\t$bp : 0x%.8X\t$sp : 0x%.8X\n", cpu->regs.rc, cpu->regs.pc, cpu->regs.bp, cpu->regs.sp);
//}

void fde_cpu(xvm_cpu *cpu, xvm_bin *bin){
    u32 instr_size = 0;
    while (get_RF(cpu)){
        // show_registers(cpu);
        instr_size = do_execute(cpu, bin);
    }
}

void fini_xvm_cpu(xvm_cpu * cpu){
    memset(cpu, 0, sizeof(xvm_cpu));
    free(cpu); cpu = NULL;
}