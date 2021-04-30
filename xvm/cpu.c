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
    cpu->flags.flags &= ~(1 << XVM_RF) | (bit << XVM_RF);
    return cpu->flags.flags & (1 << XVM_RF);
}

u8 set_ZF(xvm_cpu * cpu, u8 bit){
    cpu->flags.flags &= ~(1 << XVM_ZF) | (bit << XVM_ZF);
    return cpu->flags.flags & (1 << XVM_ZF);
}

u8 set_CF(xvm_cpu * cpu, u8 bit){
    cpu->flags.flags &= ~(1 << XVM_CF) | (bit << XVM_CF);
    return cpu->flags.flags & (1 << XVM_CF);
}

xvm_cpu * init_xvm_cpu(){
    xvm_cpu * cpu = (xvm_cpu *)malloc(sizeof(xvm_cpu));
    reset_reg(&cpu->regs);
    reset_flags(&cpu->flags);

    return cpu;
}

void fde_cpu(xvm_cpu *cpu, xvm_bin *bin){
    u32 instr_size = 0;
    while (get_RF(cpu)){
        instr_size = do_execute(cpu, bin);
    }
}

void fini_xvm_cpu(xvm_cpu * cpu){
    memset(cpu, 0, sizeof(xvm_cpu));
    free(cpu); cpu = NULL;
}