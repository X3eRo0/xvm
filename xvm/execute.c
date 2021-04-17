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

u32 execute(xvm_cpu* cpu, xvm_bin* bin){
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

    switch(opcd){
        case XVM_OP_HLT: set_RF(cpu, 0); size = 2; break;
        case XVM_OP_NOP: size = 2; break;
        case XVM_OP_ADD: {
            size = get_argument(cpu, bin, mode, &arg1, &arg2, narg);
            *arg1 += *arg2;
            if (*arg1 == 0){
                set_ZF(cpu, 1);
                set_CF(cpu, 0);
            }
            break;
        }
        default: size = 2; break; // default NOP
    }
    return size;
}