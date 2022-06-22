//
// Created by X3eRo0 on 4/17/2021.
//

#include <cpu.h>

u32* get_register(xvm_cpu* cpu, u8 reg_id)
{
    switch (reg_id) {
    case r0:
        return &cpu->regs.r0;
    case r1:
        return &cpu->regs.r1;
    case r2:
        return &cpu->regs.r2;
    case r3:
        return &cpu->regs.r3;
    case r4:
        return &cpu->regs.r4;
    case r5:
        return &cpu->regs.r5;
    case r6:
        return &cpu->regs.r6;
    case r7:
        return &cpu->regs.r7;
    case r8:
        return &cpu->regs.r8;
    case r9:
        return &cpu->regs.r9;
    case ra:
        return &cpu->regs.ra;
    case rb:
        return &cpu->regs.rb;
    case rc:
        return &cpu->regs.rc;
    case pc:
        return &cpu->regs.pc;
    case bp:
        return &cpu->regs.bp;
    case sp:
        return &cpu->regs.sp;
    default: {
        raise_signal(cpu->errors, XSIGILL, cpu->regs.pc, 0);
        // fprintf(stderr, "[" KRED "-" KNRM "] Invalid Register\n");
        return NULL;
    };
    }
}

u32 load_effective_address(xvm_cpu* cpu, xvm_bin* bin, u8 mode, u32** arg1, u32* arg2)
{
    u32 size = 2; // opcode, mode
    u8 mode1 = get_mode1(mode);
    u8 mode2 = get_mode2(mode);
    u32* temp = NULL;

    if (!mode1 && mode2) {
        // invalid mode;
        raise_signal(cpu->errors, XSIGILL, cpu->regs.pc, 0);
        return E_ERR;
        // fprintf(stderr, "[" KRED "-" KNRM "] Invalid Mode Byte\n");
        // exit(-1);
    }

    if (mode1 != XVM_REGD) {
        raise_signal(cpu->errors, XSIGILL, cpu->regs.pc, 0);
        return E_ERR;
    }

    if ((temp = get_register(cpu, read_byte(bin->x_section, cpu->regs.pc++, PERM_EXEC))) == NULL) {
        return E_ERR;
    }

    *arg1 = temp;
    size += sizeof(u8);

    if (mode2 != XVM_REGD && mode2 != XVM_IMMD) {
        u32 reg_ptr = 0;
        if (mode2 & XVM_REGD) { // pointer has a register as base at least
            u8 reg_byte = read_byte(bin->x_section, cpu->regs.pc, PERM_EXEC);

            if ((temp = get_register(cpu, reg_byte)) == NULL) {
                return E_ERR;
            }
            if (reg_byte == pc) {
                reg_ptr = *temp - 3;
            } else {
                reg_ptr = *temp;
            }
            *arg2 = reg_ptr;
            cpu->regs.pc++;
            size += sizeof(u8);
        }
        if (mode2 & XVM_IMMD) { // pointer has an immediate offset also

            if ((temp = get_reference(bin->x_section, cpu->regs.pc, PERM_EXEC)) == NULL) {
                return E_ERR;
            }
            cpu->regs.pc += sizeof(u32);
            *arg2 = reg_ptr + *temp;
            size += sizeof(u32);
        }
    } else {
        // invalid mode;
        raise_signal(cpu->errors, XSIGILL, cpu->regs.pc, 0);
        return E_ERR;
    }

    return size;
}

u32 get_argument(xvm_cpu* cpu, xvm_bin* bin, u8 mode, u32** arg1, u32** arg2)
{
    u32 size = 2; // opcode, mode
    u8 mode1 = get_mode1(mode);
    u8 mode2 = get_mode2(mode);
    u32* temp = NULL;

    if (!mode1 && mode2) {
        // invalid mode;
        raise_signal(cpu->errors, XSIGILL, cpu->regs.pc, 0);
        return E_ERR;
        // fprintf(stderr, "[" KRED "-" KNRM "] Invalid Mode Byte\n");
        // exit(-1);
    }

    if (mode1) {
        switch (mode1) {
        case XVM_REGD: {
            if ((temp = get_register(cpu, read_byte(bin->x_section, cpu->regs.pc++, PERM_EXEC))) == NULL) {
                return E_ERR;
            }

            *arg1 = temp;
            size += sizeof(u8);
            break;
        }
        case XVM_IMMD: {
            if ((temp = get_reference(bin->x_section, cpu->regs.pc, PERM_EXEC)) == NULL) {
                return E_ERR;
            }

            *arg1 = temp;
            cpu->regs.pc += sizeof(u32);
            size += sizeof(u32);
            break;
        }
        default: { // when an argument is a pointer
            if (mode1 & XVM_PTRD) {
                u32 reg_ptr = 0;
                u32 immd = 0;
                if (mode1 & XVM_REGD) { // pointer has a register as base at least
                    if ((temp = get_register(cpu, read_byte(bin->x_section, cpu->regs.pc, PERM_EXEC))) == NULL) {
                        return E_ERR;
                    }
                    reg_ptr = *temp;
                    if ((temp = get_reference(bin->x_section, reg_ptr, PERM_WRITE)) == NULL) {
                        return E_ERR;
                    }
                    *arg1 = temp;
                    cpu->regs.pc++;
                    size += sizeof(u8);
                }
                if (mode1 & XVM_IMMD) { // pointer has an immediate offset also

                    if ((temp = get_reference(bin->x_section, cpu->regs.pc, PERM_EXEC)) == NULL) {
                        return E_ERR;
                    }
                    immd = *temp;
                    cpu->regs.pc += sizeof(u32);
                    if ((temp = get_reference(bin->x_section, reg_ptr + immd, PERM_WRITE)) == NULL) {
                        return E_ERR;
                    }
                    *arg1 = temp;
                    size += sizeof(u32);
                }
                break;
            } else {
                // invalid mode;
                raise_signal(cpu->errors, XSIGILL, cpu->regs.pc, 0);
                return E_ERR;
                // fprintf(stderr, "[" KRED "-" KNRM "] Invalid Mode Byte\n");
                // exit(-1);
            }
        }
        }
    }

    if (mode2) {
        switch (mode2) {
        case XVM_REGD: {
            if ((temp = get_register(cpu, read_byte(bin->x_section, cpu->regs.pc++, PERM_EXEC))) == NULL) {
                return E_ERR;
            }
            *arg2 = temp;
            size += sizeof(u8);
            break;
        }
        case XVM_IMMD: {
            if ((temp = get_reference(bin->x_section, cpu->regs.pc, PERM_EXEC)) == NULL) {
                return E_ERR;
            }
            *arg2 = temp;
            cpu->regs.pc += sizeof(u32);
            size += sizeof(u32);
            break;
        }
        default: { // when an argument is a pointer
            if (mode2 & XVM_PTRD) {
                u32 reg_ptr = 0;
                u32 immd = 0;
                if (mode2 & XVM_REGD) { // pointer has a register as base at least
                    if ((temp = get_register(cpu, read_byte(bin->x_section, cpu->regs.pc, PERM_EXEC))) == NULL) {
                        return E_ERR;
                    }
                    reg_ptr = *temp;
                    if ((temp = get_reference(bin->x_section, reg_ptr, PERM_READ)) == NULL) {
                        return E_ERR;
                    }
                    *arg2 = temp;
                    cpu->regs.pc++;
                    size += sizeof(u8);
                }
                if (mode2 & XVM_IMMD) { // pointer has an immediate offset also
                    if ((temp = get_reference(bin->x_section, cpu->regs.pc, PERM_EXEC)) == NULL) {
                        return E_ERR;
                    }
                    immd = *temp;
                    cpu->regs.pc += sizeof(u32);
                    if ((temp = get_reference(bin->x_section, reg_ptr + immd, PERM_READ)) == NULL) {
                        return E_ERR;
                    }
                    *arg2 = temp;
                    size += sizeof(u32);
                }
                break;
            } else {
                // invalid mode;
                raise_signal(cpu->errors, XSIGILL, cpu->regs.pc, 0);
                return E_ERR;
                // fprintf(stderr, "[" KRED "-" KNRM "] Invalid Mode Byte\n");
                // exit(-1);
            }
        }
        }
    }

    return size;
}

u32 do_execute(xvm_cpu* cpu, xvm_bin* bin)
{
    u32* arg1 = NULL; // arguments will be returned here
    u32* arg2 = NULL; // arguments will be returned here
    u8 opcd = 0;
    u8 mode = 0;
    u32 size = 0;
    if ((opcd = read_byte(bin->x_section, cpu->regs.pc++, PERM_EXEC)) == (u8)E_ERR) {
        return E_ERR;
    }
    if ((mode = read_byte(bin->x_section, cpu->regs.pc++, PERM_EXEC)) == (u8)E_ERR) {
        return E_ERR;
    }
    // resolve arguments

    if (opcd != XVM_OP_LEA) {
        size = get_argument(cpu, bin, mode, &arg1, &arg2);
    }

    if (size == E_ERR) {
        return E_ERR;
    }

    switch (opcd) {

    // hlt
    case XVM_OP_HLT:
        set_RF(cpu, 0);
        size = 2;
        break;

    // nop
    case XVM_OP_NOP:
        size = 2;
        break;

    // syscall
    case XVM_OP_SYSC: {
        do_syscall(cpu, bin);
        break;
    }
    // trap
    case XVM_OP_TRAP: {
        raise_signal(cpu->errors, XSIGTRAP, cpu->regs.pc, 0);
        break;
    }

    // mov
    case XVM_OP_MOV: {

        if (!arg1 || !arg2) {
            raise_signal(cpu->errors, XSIGILL, cpu->regs.pc, 0);
            return E_ERR;
        }

        *arg1 = *arg2;
        break;
    }

    case XVM_OP_MOVB: {

        if (!arg1 || !arg2) {
            raise_signal(cpu->errors, XSIGILL, cpu->regs.pc, 0);
            return E_ERR;
        }

        *(u8*)arg1 = *(u8*)arg2;
        break;
    }

    case XVM_OP_MOVW: {

        if (!arg1 || !arg2) {
            raise_signal(cpu->errors, XSIGILL, cpu->regs.pc, 0);
            return E_ERR;
        }

        *(u16*)arg1 = *(u16*)arg2;
        break;
    }

    case XVM_OP_CMOVZ:
    case XVM_OP_CMOVE: {

        if (!arg1 || !arg2) {
            raise_signal(cpu->errors, XSIGILL, cpu->regs.pc, 0);
            return E_ERR;
        }

        if (get_ZF(cpu)) {
            *arg1 = *arg2;
        }
        break;
    }
    case XVM_OP_CMOVZW:
    case XVM_OP_CMOVEW: {

        if (!arg1 || !arg2) {
            raise_signal(cpu->errors, XSIGILL, cpu->regs.pc, 0);
            return E_ERR;
        }
        if (get_ZF(cpu)) {
            *(u16*)arg1 = *(u16*)arg2;
        }
        break;
    }
    case XVM_OP_CMOVZB:
    case XVM_OP_CMOVEB: {

        if (!arg1 || !arg2) {
            raise_signal(cpu->errors, XSIGILL, cpu->regs.pc, 0);
            return E_ERR;
        }
        if (get_ZF(cpu)) {
            *(u8*)arg1 = *(u8*)arg2;
        }
        break;
    }

    case XVM_OP_CMOVNZ:
    case XVM_OP_CMOVNE: {

        if (!arg1 || !arg2) {
            raise_signal(cpu->errors, XSIGILL, cpu->regs.pc, 0);
            return E_ERR;
        }

        if (!get_ZF(cpu)) {
            *arg1 = *arg2;
        }
        break;
    }
    case XVM_OP_CMOVNZW:
    case XVM_OP_CMOVNEW: {

        if (!arg1 || !arg2) {
            raise_signal(cpu->errors, XSIGILL, cpu->regs.pc, 0);
            return E_ERR;
        }
        if (!get_ZF(cpu)) {
            *(u16*)arg1 = *(u16*)arg2;
        }
        break;
    }
    case XVM_OP_CMOVNZB:
    case XVM_OP_CMOVNEB: {

        if (!arg1 || !arg2) {
            raise_signal(cpu->errors, XSIGILL, cpu->regs.pc, 0);
            return E_ERR;
        }
        if (!get_ZF(cpu)) {
            *(u8*)arg1 = *(u8*)arg2;
        }
        break;
    }

    case XVM_OP_CMOVA: {

        if (!arg1 || !arg2) {
            raise_signal(cpu->errors, XSIGILL, cpu->regs.pc, 0);
            return E_ERR;
        }

        if (!get_ZF(cpu) && !get_CF(cpu)) {
            *arg1 = *arg2;
        }
        break;
    }
    case XVM_OP_CMOVAW: {

        if (!arg1 || !arg2) {
            raise_signal(cpu->errors, XSIGILL, cpu->regs.pc, 0);
            return E_ERR;
        }
        if (!get_ZF(cpu) && !get_CF(cpu)) {
            *(u16*)arg1 = *(u16*)arg2;
        }
        break;
    }
    case XVM_OP_CMOVAB: {

        if (!arg1 || !arg2) {
            raise_signal(cpu->errors, XSIGILL, cpu->regs.pc, 0);
            return E_ERR;
        }
        if (!get_ZF(cpu) && !get_CF(cpu)) {
            *(u8*)arg1 = *(u8*)arg2;
        }
        break;
    }

    case XVM_OP_CMOVAE: {

        if (!arg1 || !arg2) {
            raise_signal(cpu->errors, XSIGILL, cpu->regs.pc, 0);
            return E_ERR;
        }

        if (!get_CF(cpu)) {
            *arg1 = *arg2;
        }
        break;
    }
    case XVM_OP_CMOVAEW: {

        if (!arg1 || !arg2) {
            raise_signal(cpu->errors, XSIGILL, cpu->regs.pc, 0);
            return E_ERR;
        }
        if (!get_CF(cpu)) {
            *(u16*)arg1 = *(u16*)arg2;
        }
        break;
    }
    case XVM_OP_CMOVAEB: {

        if (!arg1 || !arg2) {
            raise_signal(cpu->errors, XSIGILL, cpu->regs.pc, 0);
            return E_ERR;
        }
        if (!get_CF(cpu)) {
            *(u8*)arg1 = *(u8*)arg2;
        }
        break;
    }

    case XVM_OP_CMOVB: {

        if (!arg1 || !arg2) {
            raise_signal(cpu->errors, XSIGILL, cpu->regs.pc, 0);
            return E_ERR;
        }

        if (!get_ZF(cpu) && get_CF(cpu)) {
            *arg1 = *arg2;
        }
        break;
    }
    case XVM_OP_CMOVBW: {

        if (!arg1 || !arg2) {
            raise_signal(cpu->errors, XSIGILL, cpu->regs.pc, 0);
            return E_ERR;
        }
        if (!get_ZF(cpu) && get_CF(cpu)) {
            *(u16*)arg1 = *(u16*)arg2;
        }
        break;
    }
    case XVM_OP_CMOVBB: {

        if (!arg1 || !arg2) {
            raise_signal(cpu->errors, XSIGILL, cpu->regs.pc, 0);
            return E_ERR;
        }
        if (!get_ZF(cpu) && get_CF(cpu)) {
            *(u8*)arg1 = *(u8*)arg2;
        }
        break;
    }

    case XVM_OP_CMOVBE: {

        if (!arg1 || !arg2) {
            raise_signal(cpu->errors, XSIGILL, cpu->regs.pc, 0);
            return E_ERR;
        }

        if (get_ZF(cpu) || get_CF(cpu)) {
            *arg1 = *arg2;
        }
        break;
    }
    case XVM_OP_CMOVBEW: {

        if (!arg1 || !arg2) {
            raise_signal(cpu->errors, XSIGILL, cpu->regs.pc, 0);
            return E_ERR;
        }
        if (get_ZF(cpu) || get_CF(cpu)) {
            *(u16*)arg1 = *(u16*)arg2;
        }
        break;
    }
    case XVM_OP_CMOVBEB: {

        if (!arg1 || !arg2) {
            raise_signal(cpu->errors, XSIGILL, cpu->regs.pc, 0);
            return E_ERR;
        }
        if (get_ZF(cpu) || get_CF(cpu)) {
            *(u8*)arg1 = *(u8*)arg2;
        }
        break;
    }

    case XVM_OP_LEA: {
        u32 temp = 0;
        size = load_effective_address(cpu, bin, mode, &arg1, &temp);
        *arg1 = temp;

        break;
    }

    // call
    case XVM_OP_CALL: {

        if (!arg1) {
            raise_signal(cpu->errors, XSIGILL, cpu->regs.pc, 0);
            return E_ERR;
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

    // lsi
    case XVM_OP_LSU: {
        if (!arg1 || !arg2) {
            raise_signal(cpu->errors, XSIGILL, cpu->regs.pc, 0);
            return E_ERR;
        }
        *arg1 = *arg1 << *arg2;

        if (*arg1 == 0) {
            set_ZF(cpu, 1);
            set_CF(cpu, 0);
        } else {
            set_ZF(cpu, 0);
            set_CF(cpu, 0);
        }
        break;
    }

    // rsi
    case XVM_OP_RSU: {
        if (!arg1 || !arg2) {
            raise_signal(cpu->errors, XSIGILL, cpu->regs.pc, 0);
            return E_ERR;
        }
        *arg1 = *arg1 >> *arg2;

        if (*arg1 == 0) {
            set_ZF(cpu, 1);
            set_CF(cpu, 0);
        } else {
            set_ZF(cpu, 0);
            set_CF(cpu, 0);
        }
        break;
    }

    // xor
    case XVM_OP_XOR: {

        if (!arg1 || !arg2) {
            raise_signal(cpu->errors, XSIGILL, cpu->regs.pc, 0);
            return E_ERR;
        }

        *arg1 ^= *arg2;
        if (*arg1 == 0) {
            set_ZF(cpu, 1);
            set_CF(cpu, 0);
        } else {
            set_ZF(cpu, 0);
            set_CF(cpu, 0);
        }
        break;
    }

    case XVM_OP_XORB: {

        if (!arg1 || !arg2) {
            raise_signal(cpu->errors, XSIGILL, cpu->regs.pc, 0);
            return E_ERR;
            // raise_signal(cpu->errors, XSIGILL, cpu->regs.pc, 0);
            return E_ERR;
        }

        *(u8*)arg1 ^= *(u8*)arg2;
        if (*(u8*)arg1 == 0) {
            set_ZF(cpu, 1);
            set_CF(cpu, 0);
        } else {
            set_ZF(cpu, 0);
            set_CF(cpu, 0);
        }
        break;
    }

    case XVM_OP_XORW: {

        if (!arg1 || !arg2) {
            raise_signal(cpu->errors, XSIGILL, cpu->regs.pc, 0);
            return E_ERR;
        }

        *(u16*)arg1 ^= *(u16*)arg2;
        if (*(u16*)arg1 == 0) {
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

        if (!arg1 || !arg2) {
            raise_signal(cpu->errors, XSIGILL, cpu->regs.pc, 0);
            return E_ERR;
        }

        *arg1 &= *arg2;
        if (*arg1 == 0) {
            set_ZF(cpu, 1);
            set_CF(cpu, 0);
        } else {
            set_ZF(cpu, 0);
            set_CF(cpu, 0);
        }
        break;
    }

    case XVM_OP_ANDB: {

        if (!arg1 || !arg2) {
            raise_signal(cpu->errors, XSIGILL, cpu->regs.pc, 0);
            return E_ERR;
        }

        *(u8*)arg1 &= *(u8*)arg2;
        if (*(u8*)arg1 == 0) {
            set_ZF(cpu, 1);
            set_CF(cpu, 0);
        } else {
            set_ZF(cpu, 0);
            set_CF(cpu, 0);
        }
        break;
    }

    case XVM_OP_ANDW: {

        if (!arg1 || !arg2) {
            raise_signal(cpu->errors, XSIGILL, cpu->regs.pc, 0);
            return E_ERR;
        }

        *(u16*)arg1 &= *(u16*)arg2;
        if (*(u16*)arg1 == 0) {
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

        if (!arg1 || !arg2) {
            raise_signal(cpu->errors, XSIGILL, cpu->regs.pc, 0);
            return E_ERR;
        }

        *arg1 |= *arg2;
        if (*arg1 == 0) {
            set_ZF(cpu, 1);
            set_CF(cpu, 0);
        } else {
            set_ZF(cpu, 0);
            set_CF(cpu, 0);
        }
        break;
    }

    case XVM_OP_ORB: {

        if (!arg1 || !arg2) {
            raise_signal(cpu->errors, XSIGILL, cpu->regs.pc, 0);
            return E_ERR;
        }

        *(u8*)arg1 |= *(u8*)arg2;
        if (*(u8*)arg1 == 0) {
            set_ZF(cpu, 1);
            set_CF(cpu, 0);
        } else {
            set_ZF(cpu, 0);
            set_CF(cpu, 0);
        }
        break;
    }

    case XVM_OP_ORW: {

        if (!arg1 || !arg2) {
            raise_signal(cpu->errors, XSIGILL, cpu->regs.pc, 0);
            return E_ERR;
        }

        *(u16*)arg1 |= *(u16*)arg2;
        if (*(u16*)arg1 == 0) {
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

        if (!arg1) {
            raise_signal(cpu->errors, XSIGILL, cpu->regs.pc, 0);
            return E_ERR;
        }

        *arg1 = ~*arg1;
        if (*arg1 == 0) {
            set_ZF(cpu, 1);
            set_CF(cpu, 0);
        } else {
            set_ZF(cpu, 0);
            set_CF(cpu, 0);
        }
        break;
    }

    case XVM_OP_NOTB: {

        if (!arg1) {
            raise_signal(cpu->errors, XSIGILL, cpu->regs.pc, 0);
            return E_ERR;
        }

        *(u8*)arg1 = ~*(u8*)arg1;
        if (*(u8*)arg1 == 0) {
            set_ZF(cpu, 1);
            set_CF(cpu, 0);
        } else {
            set_ZF(cpu, 0);
            set_CF(cpu, 0);
        }
        break;
    }

    case XVM_OP_NOTW: {

        if (!arg1) {
            raise_signal(cpu->errors, XSIGILL, cpu->regs.pc, 0);
            return E_ERR;
        }

        *(u16*)arg1 = ~*(u16*)arg1;
        if (*(u16*)arg1 == 0) {
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

        if (!arg1 || !arg2) {
            raise_signal(cpu->errors, XSIGILL, cpu->regs.pc, 0);
            return E_ERR;
        }

        *arg1 += *arg2;
        if (*arg1 == 0) {
            set_ZF(cpu, 1);
            set_CF(cpu, 0);
        } else {
            set_ZF(cpu, 0);
            set_CF(cpu, 0);
        }
        break;
    }

    case XVM_OP_ADDB: {

        if (!arg1 || !arg2) {
            raise_signal(cpu->errors, XSIGILL, cpu->regs.pc, 0);
            return E_ERR;
        }

        *(u8*)arg1 += *(u8*)arg2;
        if (*(u8*)arg1 == 0) {
            set_ZF(cpu, 1);
            set_CF(cpu, 0);
        } else {
            set_ZF(cpu, 0);
            set_CF(cpu, 0);
        }
        break;
    }

    case XVM_OP_ADDW: {

        if (!arg1 || !arg2) {
            raise_signal(cpu->errors, XSIGILL, cpu->regs.pc, 0);
            return E_ERR;
        }

        *(u16*)arg1 += *(u16*)arg2;
        if (*(u16*)arg1 == 0) {
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

        if (!arg1 || !arg2) {
            raise_signal(cpu->errors, XSIGILL, cpu->regs.pc, 0);
            return E_ERR;
        }

        *arg1 -= *arg2;
        if (*arg1 == 0) {
            set_ZF(cpu, 1);
            set_CF(cpu, 0);
        } else {
            set_ZF(cpu, 0);
            set_CF(cpu, 0);
        }
        break;
    }

    case XVM_OP_SUBB: {

        if (!arg1 || !arg2) {
            raise_signal(cpu->errors, XSIGILL, cpu->regs.pc, 0);
            return E_ERR;
        }

        *(u8*)arg1 -= *(u8*)arg2;
        if (*(u8*)arg1 == 0) {
            set_ZF(cpu, 1);
            set_CF(cpu, 0);
        } else {
            set_ZF(cpu, 0);
            set_CF(cpu, 0);
        }
        break;
    }

    case XVM_OP_SUBW: {

        if (!arg1 || !arg2) {
            raise_signal(cpu->errors, XSIGILL, cpu->regs.pc, 0);
            return E_ERR;
        }

        *(u16*)arg1 -= *(u16*)arg2;
        if (*(u16*)arg1 == 0) {
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

        if (!arg1 || !arg2) {
            raise_signal(cpu->errors, XSIGILL, cpu->regs.pc, 0);
            return E_ERR;
        }

        *arg1 *= *arg2;
        if (*arg1 == 0) {
            set_ZF(cpu, 1);
            set_CF(cpu, 0);
        } else {
            set_ZF(cpu, 0);
            set_CF(cpu, 0);
        }
        break;
    }

    case XVM_OP_MULB: {

        if (!arg1 || !arg2) {
            raise_signal(cpu->errors, XSIGILL, cpu->regs.pc, 0);
            return E_ERR;
        }

        *(u8*)arg1 *= *(u8*)arg2;
        if (*(u8*)arg1 == 0) {
            set_ZF(cpu, 1);
            set_CF(cpu, 0);
        } else {
            set_ZF(cpu, 0);
            set_CF(cpu, 0);
        }
        break;
    }

    case XVM_OP_MULW: {

        if (!arg1 || !arg2) {
            raise_signal(cpu->errors, XSIGILL, cpu->regs.pc, 0);
            return E_ERR;
        }

        *(u16*)arg1 *= *(u16*)arg2;
        if (*(u16*)arg1 == 0) {
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

        if (!arg1 || !arg2) {
            raise_signal(cpu->errors, XSIGILL, cpu->regs.pc, 0);
            return E_ERR;
        }

        if (*arg2 == 0) {
            raise_signal(cpu->errors, XSIGFPE, cpu->regs.pc, 0);
            return E_ERR;
        }

        u32 modulo = *arg1 % *arg2;
        *arg1 /= *arg2;
        cpu->regs.r5 = modulo;

        if (*arg1 == 0) {
            set_ZF(cpu, 1);
            set_CF(cpu, 0);
        } else {
            set_ZF(cpu, 0);
            set_CF(cpu, 0);
        }
        break;
    }

    case XVM_OP_DIVB: {

        if (!arg1 || !arg2) {
            raise_signal(cpu->errors, XSIGILL, cpu->regs.pc, 0);
            return E_ERR;
        }

        if (*(u8*)arg2 == 0) {
            raise_signal(cpu->errors, XSIGFPE, cpu->regs.pc, 0);
            return E_ERR;
        }

        u32 modulo = *(u8*)arg1 % *(u8*)arg2;
        *(u8*)arg1 /= *(u8*)arg2;
        cpu->regs.r5 = modulo;

        if (*(u8*)arg1 == 0) {
            set_ZF(cpu, 1);
            set_CF(cpu, 0);
        } else {
            set_ZF(cpu, 0);
            set_CF(cpu, 0);
        }
        break;
    }

    case XVM_OP_DIVW: {

        if (!arg1 || !arg2) {
            raise_signal(cpu->errors, XSIGILL, cpu->regs.pc, 0);
            return E_ERR;
        }

        if (*(u16*)arg2 == 0) {
            raise_signal(cpu->errors, XSIGFPE, cpu->regs.pc, 0);
            return E_ERR;
        }

        u32 modulo = *(u16*)arg1 % *(u16*)arg2;
        *(u16*)arg1 /= *(u16*)arg2;
        cpu->regs.r5 = modulo;

        if (*(u16*)arg1 == 0) {
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

        if (!arg1) {
            raise_signal(cpu->errors, XSIGILL, cpu->regs.pc, 0);
            return E_ERR;
        }

        cpu->regs.sp -= sizeof(u32);
        write_dword(bin->x_section, cpu->regs.sp, *arg1);
        break;
    }

    case XVM_OP_PUSHA: {

        cpu->regs.sp -= sizeof(u32);
        write_dword(bin->x_section, cpu->regs.sp, cpu->regs.r0);
        cpu->regs.sp -= sizeof(u32);
        write_dword(bin->x_section, cpu->regs.sp, cpu->regs.r1);
        cpu->regs.sp -= sizeof(u32);
        write_dword(bin->x_section, cpu->regs.sp, cpu->regs.r2);
        cpu->regs.sp -= sizeof(u32);
        write_dword(bin->x_section, cpu->regs.sp, cpu->regs.r3);
        cpu->regs.sp -= sizeof(u32);
        write_dword(bin->x_section, cpu->regs.sp, cpu->regs.r4);
        cpu->regs.sp -= sizeof(u32);
        write_dword(bin->x_section, cpu->regs.sp, cpu->regs.r5);
        cpu->regs.sp -= sizeof(u32);
        write_dword(bin->x_section, cpu->regs.sp, cpu->regs.r6);
        cpu->regs.sp -= sizeof(u32);
        write_dword(bin->x_section, cpu->regs.sp, cpu->regs.r7);
        cpu->regs.sp -= sizeof(u32);
        write_dword(bin->x_section, cpu->regs.sp, cpu->regs.r8);
        cpu->regs.sp -= sizeof(u32);
        write_dword(bin->x_section, cpu->regs.sp, cpu->regs.r9);
        cpu->regs.sp -= sizeof(u32);
        write_dword(bin->x_section, cpu->regs.sp, cpu->regs.ra);
        cpu->regs.sp -= sizeof(u32);
        write_dword(bin->x_section, cpu->regs.sp, cpu->regs.rb);
        cpu->regs.sp -= sizeof(u32);
        write_dword(bin->x_section, cpu->regs.sp, cpu->regs.rc);
        break;
    }

    // pop
    case XVM_OP_POP: {

        *arg1 = read_dword(bin->x_section, cpu->regs.sp, PERM_WRITE);
        cpu->regs.sp += sizeof(u32);
        break;
    }

    case XVM_OP_POPA: {

        if (!arg1) {
            raise_signal(cpu->errors, XSIGILL, cpu->regs.pc, 0);
            return E_ERR;
        }

        cpu->regs.r0 = read_dword(bin->x_section, cpu->regs.sp, PERM_WRITE);
        cpu->regs.sp += sizeof(u32);
        cpu->regs.r1 = read_dword(bin->x_section, cpu->regs.sp, PERM_WRITE);
        cpu->regs.sp += sizeof(u32);
        cpu->regs.r2 = read_dword(bin->x_section, cpu->regs.sp, PERM_WRITE);
        cpu->regs.sp += sizeof(u32);
        cpu->regs.r3 = read_dword(bin->x_section, cpu->regs.sp, PERM_WRITE);
        cpu->regs.sp += sizeof(u32);
        cpu->regs.r4 = read_dword(bin->x_section, cpu->regs.sp, PERM_WRITE);
        cpu->regs.sp += sizeof(u32);
        cpu->regs.r5 = read_dword(bin->x_section, cpu->regs.sp, PERM_WRITE);
        cpu->regs.sp += sizeof(u32);
        cpu->regs.r6 = read_dword(bin->x_section, cpu->regs.sp, PERM_WRITE);
        cpu->regs.sp += sizeof(u32);
        cpu->regs.r7 = read_dword(bin->x_section, cpu->regs.sp, PERM_WRITE);
        cpu->regs.sp += sizeof(u32);
        cpu->regs.r8 = read_dword(bin->x_section, cpu->regs.sp, PERM_WRITE);
        cpu->regs.sp += sizeof(u32);
        cpu->regs.r9 = read_dword(bin->x_section, cpu->regs.sp, PERM_WRITE);
        cpu->regs.sp += sizeof(u32);
        cpu->regs.ra = read_dword(bin->x_section, cpu->regs.sp, PERM_WRITE);
        cpu->regs.sp += sizeof(u32);
        cpu->regs.rb = read_dword(bin->x_section, cpu->regs.sp, PERM_WRITE);
        cpu->regs.sp += sizeof(u32);
        cpu->regs.rc = read_dword(bin->x_section, cpu->regs.sp, PERM_WRITE);
        cpu->regs.sp += sizeof(u32);
        break;
    }

    // xchg
    case XVM_OP_XCHG: {

        if (!arg1 || !arg2) {
            raise_signal(cpu->errors, XSIGILL, cpu->regs.pc, 0);
            return E_ERR;
        }

        u32 temp = *arg1;
        *arg1 = *arg2;
        *arg2 = temp;
        break;
    }

    // inc
    case XVM_OP_INC: {

        if (!arg1) {
            raise_signal(cpu->errors, XSIGILL, cpu->regs.pc, 0);
            return E_ERR;
        }

        (*arg1)++;

        if (*arg1 == 0) {
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

        if (!arg1) {
            raise_signal(cpu->errors, XSIGILL, cpu->regs.pc, 0);
            return E_ERR;
        }

        (*arg1)--;

        if (*arg1 == 0) {
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

        if (!arg1 || !arg2) {
            raise_signal(cpu->errors, XSIGILL, cpu->regs.pc, 0);
            return E_ERR;
        }

        u32 res = *arg1 - *arg2;

        if (res == 0) {
            set_ZF(cpu, 1);
        }

        if (*arg1 < *arg2) {
            set_ZF(cpu, 0);
            set_CF(cpu, 1);
        }
        if (*arg1 > *arg2) {
            set_ZF(cpu, 0);
            set_CF(cpu, 0);
        }

        if (res & (1 << 31)) {
            set_SF(cpu, 1);
        } else {
            set_SF(cpu, 0);
        }
        break;
    }

    case XVM_OP_CMPB: {

        if (!arg1 || !arg2) {
            raise_signal(cpu->errors, XSIGILL, cpu->regs.pc, 0);
            return E_ERR;
        }

        if (*(u8*)arg1 == *(u8*)arg2) {
            set_ZF(cpu, 1);
        }

        if (*(u8*)arg1 < *(u8*)arg2) {
            set_ZF(cpu, 0);
            set_CF(cpu, 1);
        }
        if (*(u8*)arg1 > *(u8*)arg2) {
            set_ZF(cpu, 0);
            set_CF(cpu, 0);
        }
        break;
    }

    case XVM_OP_CMPW: {

        if (!arg1 || !arg2) {
            raise_signal(cpu->errors, XSIGILL, cpu->regs.pc, 0);
            return E_ERR;
        }

        if (*(u16*)arg1 == *(u16*)arg2) {
            set_ZF(cpu, 1);
        }

        if (*(u16*)arg1 < *(u16*)arg2) {
            set_ZF(cpu, 0);
            set_CF(cpu, 1);
        }
        if (*(u16*)arg1 > *(u16*)arg2) {
            set_ZF(cpu, 0);
            set_CF(cpu, 0);
        }
        break;
    }

    // test
    case XVM_OP_TEST: {

        if (!arg1 || !arg2) {
            raise_signal(cpu->errors, XSIGILL, cpu->regs.pc, 0);
            return E_ERR;
        }

        if ((*arg1 & *arg2) == 0) {
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

        if (!arg1) {
            raise_signal(cpu->errors, XSIGILL, cpu->regs.pc, 0);
            return E_ERR;
        }

        cpu->regs.pc = *arg1;
        break;
    }

    // rjmp
    case XVM_OP_RJMP: {

        if (!arg1) {
            raise_signal(cpu->errors, XSIGILL, cpu->regs.pc, 0);
            return E_ERR;
        }

        cpu->regs.pc += (signed int)*arg1 - size;
        break;
    }

    // jz/je
    case XVM_OP_JE:
    case XVM_OP_JZ: {

        if (!arg1) {
            raise_signal(cpu->errors, XSIGILL, cpu->regs.pc, 0);
            return E_ERR;
        }

        if (get_ZF(cpu)) {
            cpu->regs.pc = *arg1;
        }

        break;
    }

    // rjz/rje
    case XVM_OP_RJE:
    case XVM_OP_RJZ: {

        if (!arg1) {
            raise_signal(cpu->errors, XSIGILL, cpu->regs.pc, 0);
            return E_ERR;
        }

        if (get_ZF(cpu)) {
            cpu->regs.pc += (signed int)*arg1 - size;
        }

        break;
    }

    // jnz/jne
    case XVM_OP_JNE:
    case XVM_OP_JNZ: {

        if (!arg1) {
            raise_signal(cpu->errors, XSIGILL, cpu->regs.pc, 0);
            return E_ERR;
        }

        if (!get_ZF(cpu)) {
            cpu->regs.pc = *arg1;
        }
        break;
    }

    // rjnz/rjne
    case XVM_OP_RJNE:
    case XVM_OP_RJNZ: {

        if (!arg1) {
            raise_signal(cpu->errors, XSIGILL, cpu->regs.pc, 0);
            return E_ERR;
        }

        if (!get_ZF(cpu)) {
            cpu->regs.pc += (signed int)*arg1 - size;
        }
        break;
    }

    // ja
    case XVM_OP_JA: {

        if (!arg1) {
            raise_signal(cpu->errors, XSIGILL, cpu->regs.pc, 0);
            return E_ERR;
        }

        if (!get_ZF(cpu) && !get_CF(cpu)) {
            cpu->regs.pc = *arg1;
        }
        break;
    }

    // jg
    case XVM_OP_JG: {

        if (!arg1) {
            raise_signal(cpu->errors, XSIGILL, cpu->regs.pc, 0);
            return E_ERR;
        }

        if (!(get_SF(cpu) || get_ZF(cpu))) {
            cpu->regs.pc = *arg1;
        }
        break;
    }

    // rja
    case XVM_OP_RJA: {

        if (!arg1) {
            raise_signal(cpu->errors, XSIGILL, cpu->regs.pc, 0);
            return E_ERR;
        }

        if (!get_ZF(cpu) && !get_CF(cpu)) {
            cpu->regs.pc += (signed int)*arg1 - size;
        }
        break;
    }

    // rjg
    case XVM_OP_RJG: {

        if (!arg1) {
            raise_signal(cpu->errors, XSIGILL, cpu->regs.pc, 0);
            return E_ERR;
        }

        if (!(get_SF(cpu) || get_ZF(cpu))) {
            cpu->regs.pc += (signed int)*arg1 - size;
        }
        break;
    }

    // jb
    case XVM_OP_JB: {

        if (!arg1) {
            raise_signal(cpu->errors, XSIGILL, cpu->regs.pc, 0);
            return E_ERR;
        }

        if (get_CF(cpu)) {
            cpu->regs.pc = *arg1;
        }
        break;
    }

    // jl
    case XVM_OP_JL: {

        if (!arg1) {
            raise_signal(cpu->errors, XSIGILL, cpu->regs.pc, 0);
            return E_ERR;
        }

        if (get_SF(cpu)) {
            cpu->regs.pc = *arg1;
        }
        break;
    }

    // rjb
    case XVM_OP_RJB: {

        if (!arg1) {
            raise_signal(cpu->errors, XSIGILL, cpu->regs.pc, 0);
            return E_ERR;
        }

        if (get_CF(cpu)) {
            cpu->regs.pc += (signed int)*arg1 - size;
        }
        break;
    }

    // rjl
    case XVM_OP_RJL: {

        if (!arg1) {
            raise_signal(cpu->errors, XSIGILL, cpu->regs.pc, 0);
            return E_ERR;
        }

        if (get_SF(cpu)) {
            cpu->regs.pc += (signed int)*arg1 - size;
        }
        break;
    }

    // jae
    case XVM_OP_JAE: {

        if (!arg1) {
            raise_signal(cpu->errors, XSIGILL, cpu->regs.pc, 0);
            return E_ERR;
        }

        if (!get_CF(cpu)) {
            cpu->regs.pc = *arg1;
        }
        break;
    }

    // rjae
    case XVM_OP_RJAE: {

        if (!arg1) {
            raise_signal(cpu->errors, XSIGILL, cpu->regs.pc, 0);
            return E_ERR;
        }

        if (!get_CF(cpu)) {
            cpu->regs.pc += (signed int)*arg1 - size;
        }
        break;
    }

    // jge
    case XVM_OP_JGE: {

        if (!arg1) {
            raise_signal(cpu->errors, XSIGILL, cpu->regs.pc, 0);
            return E_ERR;
        }

        if (!get_SF(cpu)) {
            cpu->regs.pc = *arg1;
        }
        break;
    }

    // rjge
    case XVM_OP_RJGE: {

        if (!arg1) {
            raise_signal(cpu->errors, XSIGILL, cpu->regs.pc, 0);
            return E_ERR;
        }

        if (!get_SF(cpu)) {
            cpu->regs.pc += (signed int)*arg1 - size;
        }
        break;
    }

    // jbe
    case XVM_OP_JBE: {

        if (!arg1) {
            raise_signal(cpu->errors, XSIGILL, cpu->regs.pc, 0);
            return E_ERR;
        }

        if (get_ZF(cpu) || get_CF(cpu)) {
            cpu->regs.pc = *arg1;
        }
        break;
    }

    // jle
    case XVM_OP_JLE: {

        if (!arg1) {
            raise_signal(cpu->errors, XSIGILL, cpu->regs.pc, 0);
            return E_ERR;
        }

        if (get_SF(cpu) || get_ZF(cpu)) {
            cpu->regs.pc = *arg1;
        }
        break;
    }

    // rjle
    case XVM_OP_RJLE: {

        if (!arg1) {
            raise_signal(cpu->errors, XSIGILL, cpu->regs.pc, 0);
            return E_ERR;
        }

        if (get_SF(cpu) || get_ZF(cpu)) {
            cpu->regs.pc += (signed int)*arg1 - size;
        }
        break;
    }

    // rjbe
    case XVM_OP_RJBE: {

        if (!arg1) {
            raise_signal(cpu->errors, XSIGILL, cpu->regs.pc, 0);
            return E_ERR;
        }

        if (get_ZF(cpu) || get_CF(cpu)) {
            cpu->regs.pc += (signed int)*arg1 - size;
        }
        break;
    }
    default:
        size = 2;
        break; // default NOP
    }
    return size;
}
