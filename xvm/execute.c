//
// Created by X3eRo0 on 4/17/2021.
//

// TODO: add Left and Right Shift Instructions
// TODO: add more syscalls

#include "cpu.h"

/*static xvm_instr instructions[] = {
    {XVM_OP_HLT, &xvm_op_hlt}
};*/

u32 * get_register(xvm_cpu * cpu, u8 reg_id){
    switch(reg_id){
        case r0: return &cpu->regs.r0;
        case r1: return &cpu->regs.r1;
        case r2: return &cpu->regs.r2;
        case r3: return &cpu->regs.r3;
        case r4: return &cpu->regs.r4;
        case r5: return &cpu->regs.r5;
        case r6: return &cpu->regs.r6;
        case r7: return &cpu->regs.r7;
        case r8: return &cpu->regs.r8;
        case r9: return &cpu->regs.r9;
        case ra: return &cpu->regs.ra;
        case rb: return &cpu->regs.rb;
        case rc: return &cpu->regs.rc;
        case pc: return &cpu->regs.pc;
        case bp: return &cpu->regs.bp;
        case sp: return &cpu->regs.sp;
        default: {
            fprintf(stderr, "[" KRED "-" KNRM "] Invalid Register\n");
            exit(-1);
        };
    }
}

u8 get_argument(xvm_cpu *cpu, xvm_bin *bin, u8 mode, u32 **arg1, u32 **arg2) {
    u32 size = 2; // opcode, mode
    u8  mode1 = get_mode1(mode);
    u8  mode2 = get_mode2(mode);

    if (!mode1 && mode2){
        // invalid mode;
        fprintf(stderr, "[" KRED "-" KNRM "] Invalid Mode Byte\n");
        exit(-1);
    }

    if (mode1){
        switch (mode1){
            case XVM_REGD: {
                *arg1 = get_register(cpu, read_byte(bin->x_section, cpu->regs.pc++, PERM_EXEC));
                size += sizeof(u8);
                break;
            }
            case XVM_IMMD: {
                *arg1 = get_reference(bin->x_section, cpu->regs.pc, PERM_EXEC);
                cpu->regs.pc += sizeof(u32);
                size += sizeof(u32);
                break;
            }
            default:{ // when an argument is a pointer
                if (mode1 & XVM_PTRD){
                    u32 reg_ptr = 0;
                    u32 immd = 0;
                    if (mode1 & XVM_REGD){ // pointer has a register as base at least
                        reg_ptr = *get_register(cpu, read_byte(bin->x_section, cpu->regs.pc, PERM_EXEC));
                        *arg1 = get_reference(bin->x_section, reg_ptr, PERM_WRITE);
                        cpu->regs.pc++;
                        size += sizeof(u8);
                    }
                    if (mode1 & XVM_IMMD){ // pointer has an immediate offset also
                        immd = *get_reference(bin->x_section, cpu->regs.pc, PERM_EXEC);
                        cpu->regs.pc += sizeof(u32);
                        *arg1 = get_reference(bin->x_section, reg_ptr + immd, PERM_WRITE);
                        size += sizeof(u32);
                    }
                    break;
                } else {
                    // invalid mode;
                    fprintf(stderr, "[" KRED "-" KNRM "] Invalid Mode Byte\n");
                    exit(-1);
                }
            }
        }
    }

    if (mode2){
        switch (mode2) {
            case XVM_REGD: {
                *arg2 = get_register(cpu, read_byte(bin->x_section, cpu->regs.pc++, PERM_EXEC));
                size += sizeof(u8);
                break;
            }
            case XVM_IMMD: {
                *arg2 = get_reference(bin->x_section, cpu->regs.pc, PERM_EXEC);
                cpu->regs.pc += sizeof(u32);
                size += sizeof(u32);
                break;
            }
            default:{ // when an argument is a pointer
                if (mode2 & XVM_PTRD){
                    u32 reg_ptr = 0;
                    u32 immd = 0;
                    if (mode2 & XVM_REGD){ // pointer has a register as base at least
                        reg_ptr = *get_register(cpu, read_byte(bin->x_section, cpu->regs.pc, PERM_EXEC));
                        *arg2 = get_reference(bin->x_section, reg_ptr, PERM_READ);
                        cpu->regs.pc++;
                        size += sizeof(u8);
                    }
                    if (mode2 & XVM_IMMD){ // pointer has an immediate offset also
                        immd = *get_reference(bin->x_section, cpu->regs.pc, PERM_EXEC);
                        cpu->regs.pc += sizeof(u32);
                        *arg2 = get_reference(bin->x_section, reg_ptr + immd, PERM_READ);
                        size += sizeof(u32);
                    }
                    break;
                } else {
                    // invalid mode;
                    fprintf(stderr, "[" KRED "-" KNRM "] Invalid Mode Byte\n");
                    exit(-1);
                }
            }
        }
    }

    return (u8) size;
}

u32 do_execute(xvm_cpu* cpu, xvm_bin* bin){
    u32 * arg1 = NULL; // arguments will be returned here
    u32 * arg2 = NULL; // arguments will be returned here
    u32 size = 0;
    u8  opcd = read_byte(bin->x_section, cpu->regs.pc++, PERM_EXEC);
    u8  mode = read_byte(bin->x_section, cpu->regs.pc++, PERM_EXEC);
    // resolve arguments
    size = get_argument(cpu, bin, mode, &arg1, &arg2);

    switch(opcd){

        // hlt
        case XVM_OP_HLT: set_RF(cpu, 0); size = 2; break;

        // nop
        case XVM_OP_NOP: size = 2; break;

        // syscall
        case XVM_OP_SYSC: {
            do_syscall(cpu, bin);
            break;
        }

        // mov
        case XVM_OP_MOV: {

            if (!arg1 || !arg2){
                cpu_error(XVM_ILLEGAL_INST, "FATAL", cpu->regs.pc);
            }

            *arg1 = *arg2;
            break;
        }

        case XVM_OP_MOVB: {

            if (!arg1 || !arg2){
                cpu_error(XVM_ILLEGAL_INST, "FATAL", cpu->regs.pc);
            }

            *(u8 *)arg1 = *(u8 *)arg2;
            break;
        }

        case XVM_OP_MOVW: {

            if (!arg1 || !arg2){
                cpu_error(XVM_ILLEGAL_INST, "FATAL", cpu->regs.pc);
            }

            *(u16 *)arg1 = *(u16 *)arg2;
            break;
        }

        // call
        case XVM_OP_CALL: {

            if (!arg1){
                cpu_error(XVM_ILLEGAL_INST, "FATAL", cpu->regs.pc);
            }

            // push eip
            cpu->regs.sp -= sizeof(u32);
            write_dword(bin->x_section, cpu->regs.sp, cpu->regs.pc);
            // eip = imm
            cpu->regs.pc = *arg1;
            break;
        }

        // ret
        case XVM_OP_RET: {
            // pop eip
            cpu->regs.pc = read_dword(bin->x_section, cpu->regs.sp, PERM_WRITE);
            cpu->regs.sp += sizeof(u32);

            break;
        }

        // xor
        case XVM_OP_XOR: {

            if (!arg1 || !arg2){
                cpu_error(XVM_ILLEGAL_INST, "FATAL", cpu->regs.pc);
            }

            *arg1 ^= *arg2;
            if (*arg1 == 0){
                set_ZF(cpu, 1);
                set_CF(cpu, 0);
            } else {
                set_ZF(cpu, 0);
                set_CF(cpu, 0);
            }
            break;
        }

        case XVM_OP_XORB:{

            if (!arg1 || !arg2){
                cpu_error(XVM_ILLEGAL_INST, "FATAL", cpu->regs.pc);
            }

            *(u8*)arg1 ^= *(u8*)arg2;
            if (*arg1 == 0){
                set_ZF(cpu, 1);
                set_CF(cpu, 0);
            } else {
                set_ZF(cpu, 0);
                set_CF(cpu, 0);
            }
            break;
        }

        case XVM_OP_XORW:{

            if (!arg1 || !arg2){
                cpu_error(XVM_ILLEGAL_INST, "FATAL", cpu->regs.pc);
            }

            *(u16*)arg1 ^= *(u16*)arg2;
            if (*arg1 == 0){
                set_ZF(cpu, 1);
                set_CF(cpu, 0);
            } else {
                set_ZF(cpu, 0);
                set_CF(cpu, 0);
            }
            break;
        }

        // and
        case XVM_OP_AND: {

            if (!arg1 || !arg2){
                cpu_error(XVM_ILLEGAL_INST, "FATAL", cpu->regs.pc);
            }

            *arg1 &= *arg2;
            if (*arg1 == 0){
                set_ZF(cpu, 1);
                set_CF(cpu, 0);
            } else {
                set_ZF(cpu, 0);
                set_CF(cpu, 0);
            }
            break;
        }

        case XVM_OP_ANDB: {

            if (!arg1 || !arg2){
                cpu_error(XVM_ILLEGAL_INST, "FATAL", cpu->regs.pc);
            }

            *(u8 *)arg1 &= *(u8 *)arg2;
            if (*arg1 == 0){
                set_ZF(cpu, 1);
                set_CF(cpu, 0);
            } else {
                set_ZF(cpu, 0);
                set_CF(cpu, 0);
            }
            break;
        }

        case XVM_OP_ANDW: {

            if (!arg1 || !arg2){
                cpu_error(XVM_ILLEGAL_INST, "FATAL", cpu->regs.pc);
            }

            *(u16 *)arg1 &= *(u16 *)arg2;
            if (*arg1 == 0){
                set_ZF(cpu, 1);
                set_CF(cpu, 0);
            } else {
                set_ZF(cpu, 0);
                set_CF(cpu, 0);
            }
            break;
        }

        // or
        case XVM_OP_OR: {

            if (!arg1 || !arg2){
                cpu_error(XVM_ILLEGAL_INST, "FATAL", cpu->regs.pc);
            }

            *arg1 |= *arg2;
            if (*arg1 == 0){
                set_ZF(cpu, 1);
                set_CF(cpu, 0);
            } else {
                set_ZF(cpu, 0);
                set_CF(cpu, 0);
            }
            break;
        }

        case XVM_OP_ORB: {

            if (!arg1 || !arg2){
                cpu_error(XVM_ILLEGAL_INST, "FATAL", cpu->regs.pc);
            }

            *(u8 *)arg1 |= *(u8 *)arg2;
            if (*arg1 == 0){
                set_ZF(cpu, 1);
                set_CF(cpu, 0);
            } else {
                set_ZF(cpu, 0);
                set_CF(cpu, 0);
            }
            break;
        }

        case XVM_OP_ORW: {

            if (!arg1 || !arg2){
                cpu_error(XVM_ILLEGAL_INST, "FATAL", cpu->regs.pc);
            }

            *(u16 *)arg1 |= *(u16 *)arg2;
            if (*arg1 == 0){
                set_ZF(cpu, 1);
                set_CF(cpu, 0);
            } else {
                set_ZF(cpu, 0);
                set_CF(cpu, 0);
            }
            break;
        }

        // not
        case XVM_OP_NOT: {

            if (!arg1){
                cpu_error(XVM_ILLEGAL_INST, "FATAL", cpu->regs.pc);
            }

            *arg1 = ~*arg1;
            if (*arg1 == 0){
                set_ZF(cpu, 1);
                set_CF(cpu, 0);
            } else {
                set_ZF(cpu, 0);
                set_CF(cpu, 0);
            }
            break;
        }

        case XVM_OP_NOTB: {

            if (!arg1){
                cpu_error(XVM_ILLEGAL_INST, "FATAL", cpu->regs.pc);
            }

            *(u8 *)arg1 = ~*(u8 *)arg1;
            if (*arg1 == 0){
                set_ZF(cpu, 1);
                set_CF(cpu, 0);
            } else {
                set_ZF(cpu, 0);
                set_CF(cpu, 0);
            }
            break;
        }

        case XVM_OP_NOTW: {

            if (!arg1){
                cpu_error(XVM_ILLEGAL_INST, "FATAL", cpu->regs.pc);
            }

            *(u16 *)arg1 = ~*(u16 *)arg1;
            if (*arg1 == 0){
                set_ZF(cpu, 1);
                set_CF(cpu, 0);
            } else {
                set_ZF(cpu, 0);
                set_CF(cpu, 0);
            }
            break;
        }

        // add
        case XVM_OP_ADD: {

            if (!arg1 || !arg2){
                cpu_error(XVM_ILLEGAL_INST, "FATAL", cpu->regs.pc);
            }

            *arg1 += *arg2;
            if (*arg1 == 0){
                set_ZF(cpu, 1);
                set_CF(cpu, 0);
            } else {
                set_ZF(cpu, 0);
                set_CF(cpu, 0);
            }
            break;
        }

        case XVM_OP_ADDB: {

            if (!arg1 || !arg2){
                cpu_error(XVM_ILLEGAL_INST, "FATAL", cpu->regs.pc);
            }

            *(u8 *)arg1 += *(u8 *)arg2;
            if (*arg1 == 0){
                set_ZF(cpu, 1);
                set_CF(cpu, 0);
            } else {
                set_ZF(cpu, 0);
                set_CF(cpu, 0);
            }
            break;
        }

        case XVM_OP_ADDW: {

            if (!arg1 || !arg2){
                cpu_error(XVM_ILLEGAL_INST, "FATAL", cpu->regs.pc);
            }

            *(u16 *)arg1 += *(u16 *)arg2;
            if (*arg1 == 0){
                set_ZF(cpu, 1);
                set_CF(cpu, 0);
            } else {
                set_ZF(cpu, 0);
                set_CF(cpu, 0);
            }
            break;
        }

        // sub
        case XVM_OP_SUB: {

            if (!arg1 || !arg2){
                cpu_error(XVM_ILLEGAL_INST, "FATAL", cpu->regs.pc);
            }

            *arg1 -= *arg2;
            if (*arg1 == 0){
                set_ZF(cpu, 1);
                set_CF(cpu, 0);
            } else {
                set_ZF(cpu, 0);
                set_CF(cpu, 0);
            }
            break;
        }

        case XVM_OP_SUBB: {

            if (!arg1 || !arg2){
                cpu_error(XVM_ILLEGAL_INST, "FATAL", cpu->regs.pc);
            }

            *(u8 *)arg1 -= *(u8 *)arg2;
            if (*arg1 == 0){
                set_ZF(cpu, 1);
                set_CF(cpu, 0);
            } else {
                set_ZF(cpu, 0);
                set_CF(cpu, 0);
            }
            break;
        }

        case XVM_OP_SUBW: {

            if (!arg1 || !arg2){
                cpu_error(XVM_ILLEGAL_INST, "FATAL", cpu->regs.pc);
            }

            *(u16 *)arg1 -= *(u16 *)arg2;
            if (*arg1 == 0){
                set_ZF(cpu, 1);
                set_CF(cpu, 0);
            } else {
                set_ZF(cpu, 0);
                set_CF(cpu, 0);
            }
            break;
        }

        // mul
        case XVM_OP_MUL: {

            if (!arg1 || !arg2){
                cpu_error(XVM_ILLEGAL_INST, "FATAL", cpu->regs.pc);
            }

            *arg1 *= *arg2;
            if (*arg1 == 0){
                set_ZF(cpu, 1);
                set_CF(cpu, 0);
            } else {
                set_ZF(cpu, 0);
                set_CF(cpu, 0);
            }
            break;
        }

        case XVM_OP_MULB: {

            if (!arg1 || !arg2){
                cpu_error(XVM_ILLEGAL_INST, "FATAL", cpu->regs.pc);
            }

            *(u8 *)arg1 *= *(u8 *)arg2;
            if (*arg1 == 0){
                set_ZF(cpu, 1);
                set_CF(cpu, 0);
            } else {
                set_ZF(cpu, 0);
                set_CF(cpu, 0);
            }
            break;
        }

        case XVM_OP_MULW: {

            if (!arg1 || !arg2){
                cpu_error(XVM_ILLEGAL_INST, "FATAL", cpu->regs.pc);
            }

            *(u16 *)arg1 *= *(u16 *)arg2;
            if (*arg1 == 0){
                set_ZF(cpu, 1);
                set_CF(cpu, 0);
            } else {
                set_ZF(cpu, 0);
                set_CF(cpu, 0);
            }
            break;
        }

        // div
        case XVM_OP_DIV: {

            if (!arg1 || !arg2){
                cpu_error(XVM_ILLEGAL_INST, "FATAL", cpu->regs.pc);
            }

            if (*arg2 == 0){
                cpu_error(XVM_FP_EXCEPTION, "FATAL", cpu->regs.pc);
            }

            u32 modulo = *arg1 % *arg2;
            *arg1 /= *arg2;
            cpu->regs.r5 = modulo;

            if (*arg1 == 0){
                set_ZF(cpu, 1);
                set_CF(cpu, 0);
            } else {
                set_ZF(cpu, 0);
                set_CF(cpu, 0);
            }
            break;
        }

        case XVM_OP_DIVB: {

            if (!arg1 || !arg2){
                cpu_error(XVM_ILLEGAL_INST, "FATAL", cpu->regs.pc);
            }

            if (*(u8 *)arg2 == 0){
                cpu_error(XVM_FP_EXCEPTION, "FATAL", cpu->regs.pc);
            }

            u32 modulo = *(u8 *)arg1 % *(u8 *)arg2;
            *(u8 *)arg1 /= *(u8 *)arg2;
            cpu->regs.r5 = modulo;

            if (*arg1 == 0){
                set_ZF(cpu, 1);
                set_CF(cpu, 0);
            } else {
                set_ZF(cpu, 0);
                set_CF(cpu, 0);
            }
            break;
        }

        case XVM_OP_DIVW: {

            if (!arg1 || !arg2){
                cpu_error(XVM_ILLEGAL_INST, "FATAL", cpu->regs.pc);
            }

            if (*(u16 *)arg2 == 0){
                cpu_error(XVM_FP_EXCEPTION, "FATAL", cpu->regs.pc);
            }

            u32 modulo = *(u16 *)arg1 % *(u16 *)arg2;
            *(u16 *)arg1 /= *(u16 *)arg2;
            cpu->regs.r5 = modulo;

            if (*arg1 == 0){
                set_ZF(cpu, 1);
                set_CF(cpu, 0);
            } else {
                set_ZF(cpu, 0);
                set_CF(cpu, 0);
            }
            break;
        }

        // push
        case XVM_OP_PUSH: {

            if (!arg1){
                cpu_error(XVM_ILLEGAL_INST, "FATAL", cpu->regs.pc);
            }

            cpu->regs.sp -= sizeof(u32);
            write_dword(bin->x_section, cpu->regs.sp, *arg1);
            break;
        }

        // pop
        case XVM_OP_POP: {

            if (!arg1){
                cpu_error(XVM_ILLEGAL_INST, "FATAL", cpu->regs.pc);
            }

            *arg1 = read_dword(bin->x_section, cpu->regs.sp, PERM_WRITE);
            cpu->regs.sp += sizeof(u32);
            break;
        }

        // xchg
        case XVM_OP_XCHG: {

            if (!arg1 || !arg2){
                cpu_error(XVM_ILLEGAL_INST, "FATAL", cpu->regs.pc);
            }

            u32 temp = *arg1;
            *arg1 = *arg2;
            *arg2 = temp;
            break;
        }

        // inc
        case XVM_OP_INC: {

            if (!arg1){
                cpu_error(XVM_ILLEGAL_INST, "FATAL", cpu->regs.pc);
            }

            (*arg1)++;

            if (*arg1 == 0){
                set_ZF(cpu, 1);
                set_CF(cpu, 0);
            } else {
                set_ZF(cpu, 0);
                set_CF(cpu, 0);
            }

            break;
        }

        // dec
        case XVM_OP_DEC: {

            if (!arg1){
                cpu_error(XVM_ILLEGAL_INST, "FATAL", cpu->regs.pc);
            }

            (*arg1)--;

            if (*arg1 == 0){
                set_ZF(cpu, 1);
                set_CF(cpu, 0);
            } else {
                set_ZF(cpu, 0);
                set_CF(cpu, 0);
            }

            break;
        }

        // cmp
        case XVM_OP_CMP: {

            if (!arg1 || !arg2){
                cpu_error(XVM_ILLEGAL_INST, "FATAL", cpu->regs.pc);
            }

            if (*arg1 == *arg2){
                set_ZF(cpu, 1);
            }

            if (*arg1 < *arg2) {
                set_ZF(cpu, 0);
                set_CF(cpu, 1);
            }
            if (*arg1 > *arg2){
                set_ZF(cpu, 0);
                set_CF(cpu, 0);
            }
            break;
        }

        case XVM_OP_CMPB: {

            if (!arg1 || !arg2){
                cpu_error(XVM_ILLEGAL_INST, "FATAL", cpu->regs.pc);
            }

            if (*(u8 *)arg1 == *(u8 *)arg2){
                set_ZF(cpu, 1);
            }

            if (*(u8 *)arg1 < *(u8 *)arg2) {
                set_ZF(cpu, 0);
                set_CF(cpu, 1);
            }
            if (*(u8 *)arg1 > *(u8 *)arg2){
                set_ZF(cpu, 0);
                set_CF(cpu, 0);
            }
            break;
        }

        case XVM_OP_CMPW: {

            if (!arg1 || !arg2){
                cpu_error(XVM_ILLEGAL_INST, "FATAL", cpu->regs.pc);
            }

            if (*(u16 *)arg1 == *(u16 *)arg2){
                set_ZF(cpu, 1);
            }

            if (*(u16 *)arg1 < *(u16 *)arg2) {
                set_ZF(cpu, 0);
                set_CF(cpu, 1);
            }
            if (*(u16 *)arg1 > *(u16 *)arg2){
                set_ZF(cpu, 0);
                set_CF(cpu, 0);
            }
            break;
        }

        // test
        case XVM_OP_TEST:{

            if (!arg1 || !arg2){
                cpu_error(XVM_ILLEGAL_INST, "FATAL", cpu->regs.pc);
            }

            if ((*arg1 & *arg2) == 0){
                set_ZF(cpu, 1);
                set_CF(cpu, 0);
            } else {
                set_ZF(cpu, 0);
                set_CF(cpu, 0);
            }
            break;
        }

        // jmp
        case XVM_OP_JMP: {

            if (!arg1){
                cpu_error(XVM_ILLEGAL_INST, "FATAL", cpu->regs.pc);
            }

            cpu->regs.pc = *arg1;
            break;
        }

        // jz
        case XVM_OP_JZ: {

            if (!arg1){
                cpu_error(XVM_ILLEGAL_INST, "FATAL", cpu->regs.pc);
            }

            if (get_ZF(cpu)){
                cpu->regs.pc = *arg1;
            }

            break;
        }

        // jnz
        case XVM_OP_JNZ: {

            if (!arg1){
                cpu_error(XVM_ILLEGAL_INST, "FATAL", cpu->regs.pc);
            }

            if (!get_ZF(cpu)){
                cpu->regs.pc = *arg1;
            }
            break;
        }

        // ja
        case XVM_OP_JA: {

            if (!arg1){
                cpu_error(XVM_ILLEGAL_INST, "FATAL", cpu->regs.pc);
            }

            if (!get_ZF(cpu) && !get_CF(cpu)){
                cpu->regs.pc = *arg1;
            }
            break;
        }

        // jb
        case XVM_OP_JB: {

            if (!arg1){
                cpu_error(XVM_ILLEGAL_INST, "FATAL", cpu->regs.pc);
            }

            if (!get_ZF(cpu) && get_CF(cpu)){
                cpu->regs.pc = *arg1;
            }
            break;
        }
        // jae
        case XVM_OP_JAE: {

            if (!arg1){
                cpu_error(XVM_ILLEGAL_INST, "FATAL", cpu->regs.pc);
            }

            if (!get_CF(cpu)){
                cpu->regs.pc = *arg1;
            }
            break;
        }
        // jbe
        case XVM_OP_JBE: {

            if (!arg1){
                cpu_error(XVM_ILLEGAL_INST, "FATAL", cpu->regs.pc);
            }

            if (get_ZF(cpu) || get_CF(cpu)){
                cpu->regs.pc = *arg1;
            }
            break;
        }
        default: size = 2; break; // default NOP
    }
    return size;
}