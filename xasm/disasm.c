//
// Created by X3eRo0 on 5/1/2021.
//

#include <xasm.h>


const char* registers[XVM_NREGS] = {
    [reg_r0] = "$r0",
    [reg_r1] = "$r1",
    [reg_r2] = "$r2",
    [reg_r3] = "$r3",
    [reg_r4] = "$r4",
    [reg_r5] = "$r5",
    [reg_r6] = "$r6",
    [reg_r7] = "$r7",
    [reg_r8] = "$r8",
    [reg_r9] = "$r9",
    [reg_ra] = "$ra",
    [reg_rb] = "$rb",
    [reg_rc] = "$rc",
    [reg_pc] = "$pc",
    [reg_bp] = "$bp",
    [reg_sp] = "$sp",
};

void xasm_disassemble(xvm_bin* bin, section_entry* sec, u32 addr, u32 ninstr)
{

    if (sec->m_name == NULL) {
        printf("\n[" KGRN "+" KNRM "] Disassembling <Unnamed Section>\n");
    } else {
        printf("\n[" KGRN "+" KNRM "] Disassembling %s\n", sec->m_name);
    }
    printf("[" KGRN "+" KNRM "] Raw Size : %d BYTES\n", sec->m_ofst);
    printf("[" KGRN "+" KNRM "] Address  : 0x%X\n", sec->v_addr);

    // sanity check
    if (!(addr >= sec->v_addr && addr < sec->v_addr + sec->v_size)) {
        printf("[" KRED "+" KNRM "] 0x%XA Address not found\n", addr);
        return;
    }

    char* bytecode = sec->m_buff; // bytecode array
    u32 pc = addr - sec->v_addr; // pc

    u8 opcd = 0;
    u8 mode = 0;
    u8 mode1 = 0;
    u8 mode2 = 0;
    u32 imm = 0;
    char* temp = NULL;

    if (ninstr == 0) {
        ninstr = -1;
    }
    for (u32 i = 0; (pc < sec->m_ofst) && i < ninstr; i++) {

        imm = sec->v_addr + pc;
        temp = resolve_symbol_name(bin->x_symtab, imm);

        if (temp != NULL) {
            printf(KBLU "\n0x%.8X" KNRM " <" KGRN "%s" KNRM ">:\n", imm, temp);
        }

        printf(KBLU "0x%-14.8X" KNRM, imm);

        opcd = bytecode[pc++];
        mode = bytecode[pc++];

        mode1 = (mode >> 0x0) & 0xf;
        mode2 = (mode >> 0x4) & 0xf;

        printf(KCYN "%-10.7s" KNRM, mnemonics[opcd]);

        /* ARG1 */
        switch (mode1) {
        case ARG_NARG:
            break;
        case ARG_REGD: {
            printf("%s", registers[bytecode[pc++]]);
            break;
        }
        case ARG_IMMD: {
            imm = *(u32*)&bytecode[pc];
            temp = resolve_symbol_name(bin->x_symtab, imm);

            if (temp == NULL) {
                if (((signed int)imm) < 0) {
                    printf(KMAG "#0x%x" KNRM, -1 * imm);
                } else {
                    printf(KMAG "#0x%x" KNRM, (signed int)imm);
                }
            } else {
                printf(KGRN "%s" KNRM, temp);
            }

            pc += sizeof(u32);
            break;
        }
        default: {
            if (mode1 & ARG_PTRD) {
                printf("[");

                if (mode1 & ARG_REGD) {
                    printf("%s", registers[bytecode[pc++]]);
                }

                if ((mode1 & ARG_REGD) && (mode1 & ARG_IMMD)) {
                    imm = *(u32*)&bytecode[pc];
                    if (((signed int)imm) < 0 && imm != 0) {
                        printf(" - ");
                    } else {
                        printf(" + ");
                    }
                }

                if (mode1 & ARG_IMMD) {
                    temp = resolve_symbol_name(bin->x_symtab, imm);

                    if (temp == NULL) {
                        if (((signed int)imm) < 0) {
                            printf(KMAG "#0x%x" KNRM, -1 * imm);
                        } else {
                            printf(KMAG "#0x%x" KNRM, (signed int)imm);
                        }

                    } else {
                        printf(KGRN "%s" KNRM, temp);
                    }
                    pc += sizeof(u32);
                }
                printf("]");
            }
        }
        }

        if (mode2 != ARG_NARG) {
            printf(", ");
        }

        /* ARG2 */
        switch (mode2) {
        case ARG_NARG:
            break;
        case ARG_REGD: {
            printf("%s", registers[bytecode[pc++]]);
            break;
        }
        case ARG_IMMD: {
            imm = *(u32*)&bytecode[pc];
            temp = resolve_symbol_name(bin->x_symtab, imm);

            if (temp == NULL) {
                if (((signed int)imm) < 0) {
                    printf(KMAG "#0x%x" KNRM, -1 * imm);
                } else {
                    printf(KMAG "#0x%x" KNRM, (signed int)imm);
                }
            } else {
                printf(KGRN "%s" KNRM, temp);
            }
            pc += sizeof(u32);
            break;
        }
        default: {
            if (mode2 & ARG_PTRD) {
                printf("[");

                if (mode2 & ARG_REGD) {
                    printf("%s", registers[bytecode[pc++]]);
                }

                if ((mode2 & ARG_REGD) && (mode2 & ARG_IMMD)) {
                    imm = *(u32*)&bytecode[pc];
                    if (((signed int)imm) < 0 && imm != 0) {
                        printf(" - ");
                    } else {
                        printf(" + ");
                    }
                }

                if (mode2 & ARG_IMMD) {
                    imm = *(u32*)&bytecode[pc];
                    temp = resolve_symbol_name(bin->x_symtab, imm);

                    if (temp == NULL) {
                        if (((signed int)imm) < 0) {
                            printf(KMAG "#0x%x" KNRM, -1 * imm);
                        } else {
                            printf(KMAG "#0x%x" KNRM, (signed int)imm);
                        }

                    } else {
                        printf(KGRN "%s" KNRM, temp);
                    }
                    pc += sizeof(u32);
                }
                printf("]");
            }
        }
        }
        puts(KNRM);
    }
}

u32 xasm_disassemble_bytes(FILE* fp, xvm_bin* bin, const char* bytecode, u32 len, u32 address, u32 ninstr, u32 need_clean)
{
    // todo: check if an instruction that requires arguments
    //       has the argument modes set to NOARGS
    // need_clean : if you want only clean disassembly
    if (bytecode == NULL || len == 0) {
        for (u32 i = 0; i < ninstr; i++) {
            fprintf(fp, "0x%-14.8XCannot access memory at address 0x%x\n", address + i, address + i);
        }
        return E_ERR;
    }

    if (ninstr == 0) {
        ninstr = -1;
    }

    u32 pc = 0;
    u32 imm = 0;
    u8 mode = 0;
    u8 mode1 = 0;
    u8 mode2 = 0;
    u8 opcode = 0;
    u8 reg = 0;
    u8 rel_jmp = 0;

    char* temp = NULL;
    u8 clean_disassembly = E_OK; // if we get any weird disassembly then clean_disassembly = E_ERR:
    u32 ninstr_disassembled = 0;

    for (u32 i = 0; (pc < len) && i < ninstr; i++) {
        opcode = bytecode[pc++];

        if (opcode >= XVM_OP_RJMP && opcode <= XVM_OP_RJBE) {
            rel_jmp = 1;
        }

        mode = bytecode[pc++];
        if (opcode >= sizeof(mnemonics) / sizeof(mnemonics[0])) {
            clean_disassembly = E_ERR;
            if (need_clean != E_OK) {
                return E_ERR;
            }
            fprintf(fp, "%-10.7s\n", "(bad)");
            continue;
        }

        mode1 = (mode >> 0x0) & 0xf;
        mode2 = (mode >> 0x4) & 0xf;

        // sanity check on 2 arg instructions
        if ((inst_to_args_dict[opcode] == ARG2) && (mode1 == ARG_NARG || mode2 == ARG_NARG)) {
            return E_ERR;
        }

        // sanity check on 1 arg instructions
        if ((inst_to_args_dict[opcode] == ARG1) && (mode1 == ARG_NARG || mode2 != ARG_NARG)) {
            return E_ERR;
        }

        // sanity check on 0 arg instructions
        if ((inst_to_args_dict[opcode] == ARG0) && (mode1 != ARG_NARG || mode2 != ARG_NARG)) {
            return E_ERR;
        }

        if (mode1 != ARG_NARG && mode1 != ARG_REGD && mode1 != ARG_IMMD && !(mode1 & ARG_PTRD)) {
            clean_disassembly = E_ERR;
            if (need_clean != E_OK) {
                return E_ERR;
            }
            fprintf(fp, "%s\n", "(bad)");
            continue;
        }

        if (mode2 != ARG_NARG && mode2 != ARG_REGD && mode2 != ARG_IMMD && !(mode2 & ARG_PTRD)) {
            clean_disassembly = E_ERR;
            if (need_clean != E_OK) {
                return E_ERR;
            }
            fprintf(fp, "%s\n", "(bad)");
            continue;
        }

        fprintf(fp, "0x%-14.8X", address + pc);
        fprintf(fp, "%-10.7s", mnemonics[opcode]);
        switch (mode1) {
        case ARG_NARG:
            break;
        case ARG_REGD: {
            reg = bytecode[pc++];
            if (reg >= sizeof(registers) / sizeof(registers[0])) {
                clean_disassembly = E_ERR;
                if (need_clean != E_OK) {
                    return E_ERR;
                }
                fprintf(fp, "%s\n", "(bad)");
                continue;
            }
            fprintf(fp, "%s", registers[reg]);
            break;
        }
        case ARG_IMMD: {
            imm = *(u32*)&bytecode[pc];

            if (rel_jmp) {
                imm = address + pc - 2 + (signed int)imm;
            }

            if (bin != NULL) {
                temp = resolve_symbol_name(bin->x_symtab, imm);
            }

            if (temp == NULL) {
                if (((signed int)imm) < 0) {
                    fprintf(fp, "#0x%x", -1 * imm);
                } else {
                    fprintf(fp, "#0x%x", (signed int)imm);
                }
            } else {
                fprintf(fp, "%s", temp);
            }
            /* --- */
            /*         imm = *(u32*)&bytecode[pc]; */
            /*         temp = resolve_symbol_name(bin->x_symtab, imm); */
            /*  */
            /*         if (temp == NULL) { */
            /*             if (((signed int)imm) < 0) { */
            /*                 printf(KMAG "#0x%x" KNRM, -1 * imm); */
            /*             } else { */
            /*                 printf(KMAG "#0x%x" KNRM, (signed int)imm); */
            /*             } */
            /*  */
            /*         } else { */
            /*             printf(KGRN "%s" KNRM, temp); */
            /*         } */
            /*         pc += sizeof(u32); */
            /* --- */

            pc += sizeof(u32);
            break;
        }
        default: {
            if (mode1 & ARG_PTRD) {
                fprintf(fp, "[");
                if (mode1 & ARG_REGD) {
                    reg = bytecode[pc++];
                    if (reg >= sizeof(registers) / sizeof(registers[0])) {
                        clean_disassembly = E_ERR;
                        if (need_clean != E_OK) {
                            return E_ERR;
                        }
                        fprintf(fp, "%s\n", "(bad)");
                        continue;
                    }
                    fprintf(fp, "%s", registers[reg]);
                }

                if ((mode1 & ARG_REGD) && (mode1 & ARG_IMMD)) {
                    imm = *(u32*)&bytecode[pc];
                    if (((signed int)imm) < 0 && imm != 0) {
                        fprintf(fp, " - ");
                    } else {
                        fprintf(fp, " + ");
                    }
                }

                if (mode1 & ARG_IMMD) {
                    imm = *(u32*)&bytecode[pc];
                    if (bin != NULL) {
                        temp = resolve_symbol_name(bin->x_symtab, imm);
                    }

                    if (temp == NULL) {
                        if (((signed int)imm) < 0) {
                            fprintf(fp, "#0x%x", -1 * imm);
                        } else {
                            fprintf(fp, "#0x%x", (signed int)imm);
                        }
                    } else {
                        fprintf(fp, "%s", temp);
                    }

                    pc += sizeof(u32);
                }
                fprintf(fp, "]");
                break;
            }
        }
        }

        if (mode2 != ARG_NARG) {
            fprintf(fp, ", ");
        }

        switch (mode2) {
        case ARG_NARG:
            break;
        case ARG_REGD: {
            reg = bytecode[pc++];
            if (reg >= sizeof(registers) / sizeof(registers[0])) {
                clean_disassembly = E_ERR;
                if (need_clean != E_OK) {
                    return E_ERR;
                }
                fprintf(fp, "%s\n", "(bad)");
                continue;
            }
            fprintf(fp, "%s", registers[reg]);
            break;
        }
        case ARG_IMMD: {
            imm = *(u32*)&bytecode[pc];
            if (bin != NULL) {
                temp = resolve_symbol_name(bin->x_symtab, imm);
            }

            if (temp == NULL) {
                if (((signed int)imm) < 0) {
                    fprintf(fp, "#0x%x", -1 * imm);
                } else {
                    fprintf(fp, "#0x%x", (signed int)imm);
                }
            } else {
                fprintf(fp, "%s", temp);
            }

            pc += sizeof(u32);
            break;
        }
        default: {
            if (mode2 & ARG_PTRD) {
                fprintf(fp, "[");
                if (mode2 & ARG_REGD) {
                    reg = bytecode[pc++];
                    if (reg >= sizeof(registers) / sizeof(registers[0])) {
                        clean_disassembly = E_ERR;
                        if (need_clean != E_OK) {
                            return E_ERR;
                        }
                        fprintf(fp, "%s\n", "(bad)");
                        continue;
                    }
                    fprintf(fp, "%s", registers[reg]);
                }

                if ((mode2 & ARG_REGD) && (mode2 & ARG_IMMD)) {
                    imm = *(u32*)&bytecode[pc];
                    if (((signed int)imm) < 0 && imm != 0) {
                        fprintf(fp, " - ");
                    } else {
                        fprintf(fp, " + ");
                    }
                }

                if (mode2 & ARG_IMMD) {
                    imm = *(u32*)&bytecode[pc];
                    if (bin != NULL) {
                        temp = resolve_symbol_name(bin->x_symtab, imm);
                    }

                    if (temp == NULL) {
                        if (((signed int)imm) < 0) {
                            fprintf(fp, "#0x%x", -1 * imm);
                        } else {
                            fprintf(fp, "#0x%x", (signed int)imm);
                        }
                    } else {
                        fprintf(fp, "%s", temp);
                    }

                    pc += sizeof(u32);
                }
                fprintf(fp, "]");
                break;
            }
        }
        }
        fprintf(fp, "\n");
        ninstr_disassembled++;
    }
    return ninstr_disassembled;
}
