//
// Created by X3eRo0 on 4/17/2021.
//

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
        case pc: return &cpu->regs.pc;
        case bp: return &cpu->regs.bp;
        case sp: return &cpu->regs.sp;
        default: return NULL;
    }
}

u8 get_argument(xvm_cpu * cpu, xvm_bin * bin, u8 mode, u32 ** arg1, u32 ** arg2, u8 narg){
    u32 size = 2; // opcode, mode
    u8  mode1 = get_mode1(mode);
    u8  mode2 = get_mode2(mode);

    if (narg > 0){
        switch (mode1){
            case XVM_NARG: break;
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
                        reg_ptr = *get_register(cpu, read_byte(bin->x_section, cpu->regs.pc++, PERM_EXEC));
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
                    printf("Invalid Mode: 0x%x\n", mode1);
                    break;
                }
            }
        }
    }

    if (narg > 1){
        switch (mode2) {
            case XVM_NARG: break;
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
                if (mode1 & XVM_PTRD){
                    u32 reg_ptr = 0;
                    u32 immd = 0;
                    if (mode1 & XVM_REGD){ // pointer has a register as base at least
                        reg_ptr = *get_register(cpu, read_byte(bin->x_section, cpu->regs.pc++, PERM_EXEC));
                        size += sizeof(u8);
                    }
                    if (mode1 & XVM_IMMD){ // pointer has an immediate offset also
                        immd = *get_reference(bin->x_section, cpu->regs.pc, PERM_EXEC);
                        cpu->regs.pc += sizeof(u32);
                        *arg2 = get_reference(bin->x_section, reg_ptr + immd, PERM_EXEC);
                        size += sizeof(u32);
                    }
                    break;
                } else {
                    // invalid mode;
                    printf("Invalid Mode: 0x%x\n", mode1);
                    break;
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
    u8  narg = 0;

    if (get_mode1(mode)){
        narg += 1;
    }

    if (get_mode2(mode)){
        narg += 1;
    }

    // resolve arguments
    size = get_argument(cpu, bin, mode, &arg1, &arg2, narg);

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
            *arg1 = *arg2;
            break;
        }

        case XVM_OP_MOVB: {
            *(u8 *)arg1 = *(u8 *)arg2;
            break;
        }

        // call
        case XVM_OP_CALL: {
            // push eip
            write_dword(bin->x_section, cpu->regs.sp, cpu->regs.pc);
            cpu->regs.sp += sizeof(u32);
            // eip = imm
            cpu->regs.pc = *arg1;
            break;
        }

        // ret
        case XVM_OP_RET: {
            // pop eip
            cpu->regs.sp -= sizeof(u32);
            cpu->regs.pc = read_dword(bin->x_section, cpu->regs.sp, PERM_WRITE);
            break;
        }

        // xor
        case XVM_OP_XOR: {
            *arg1 ^= *arg2;
            if (*arg1 == 0){
                set_ZF(cpu, 1);
                set_CF(cpu, 0);
            }
            break;
        }

        // and
        case XVM_OP_AND: {
            *arg1 &= *arg2;
            if (*arg1 == 0){
                set_ZF(cpu, 1);
                set_CF(cpu, 0);
            }
            break;
        }

        // or
        case XVM_OP_OR: {
            *arg1 |= *arg2;
            if (*arg1 == 0){
                set_ZF(cpu, 1);
                set_CF(cpu, 0);
            }
            break;
        }

        // not
        case XVM_OP_NOT: {
            *arg1 = ~*arg1;
            if (*arg1 == 0){
                set_ZF(cpu, 1);
                set_CF(cpu, 0);
            }
            break;
        }

        // add
        case XVM_OP_ADD: {
            *arg1 += *arg2;
            if (*arg1 == 0){
                set_ZF(cpu, 1);
                set_CF(cpu, 0);
            }
            break;
        }

        // sub
        case XVM_OP_SUB: {
            *arg1 -= *arg2;
            if (*arg1 == 0){
                set_ZF(cpu, 1);
                set_CF(cpu, 0);
            }
            break;
        }

        // mul
        case XVM_OP_MUL: {
            *arg1 *= *arg2;
            if (*arg1 == 0){
                set_ZF(cpu, 1);
                set_CF(cpu, 0);
            }
            break;
        }

        // div
        case XVM_OP_DIV: {

            if (arg2 == 0){
                // FIXME: Give divide by zero error
                break;
            }

            *arg1 /= *arg2;
            cpu->regs.r1 = *arg1 % *arg2;

            if (*arg1 == 0){
                set_ZF(cpu, 1);
                set_CF(cpu, 0);
            }
            break;
        }

        // push
        case XVM_OP_PUSH: {
            write_dword(bin->x_section, cpu->regs.sp, *arg1);
            cpu->regs.sp += sizeof(u32);
            break;
        }

        // pop
        case XVM_OP_POP: {
            cpu->regs.sp -= sizeof(u32);
            *arg1 = read_dword(bin->x_section, cpu->regs.sp, PERM_WRITE);
            break;
        }

        // xchg
        case XVM_OP_XCHG: {
            u32 temp = *arg1;
            *arg1 = *arg2;
            *arg2 = temp;
            break;
        }

        // inc
        case XVM_OP_INC: {
            *arg1++;

            if (*arg1 == 0){
                set_ZF(cpu, 1);
                set_CF(cpu, 0);
            }

            break;
        }

        // dec
        case XVM_OP_DEC: {
            *arg1--;

            if (*arg1 == 0){
                set_ZF(cpu, 1);
                set_CF(cpu, 0);
            }

            break;
        }

        // cmp
        case XVM_OP_CMP: {
            if (*arg1 == *arg2){
                set_ZF(cpu, 1);
            }

            if (*arg1 < *arg1){
                set_ZF(cpu, 0);
                set_CF(cpu, 1);
            }

            if (*arg1 > *arg2){
                set_ZF(cpu, 0);
                set_CF(cpu, 0);
            }
            break;
        }

        // jmp
        case XVM_OP_JMP: {
            cpu->regs.pc = *arg1;
            break;
        }

        // jz
        case XVM_OP_JZ: {
            if (get_ZF(cpu)){
                cpu->regs.pc = *arg1;
            }
            break;
        }

        // jnz
        case XVM_OP_JNZ: {
            if (!get_ZF(cpu)){
                cpu->regs.pc = *arg1;
            }
            break;
        }

        // ja
        case XVM_OP_JA: {
            if (!get_ZF(cpu) && !get_CF(cpu)){
                cpu->regs.pc = *arg1;
            }
            break;
        }

        // jb
        case XVM_OP_JB: {
            if (!get_ZF(cpu) && get_CF(cpu)){
                cpu->regs.pc = *arg1;
            }
            break;
        }
        default: size = 2; break; // default NOP
    }
    return size;
}