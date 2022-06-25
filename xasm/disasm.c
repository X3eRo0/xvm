//
// Created by X3eRo0 on 5/1/2021.
//

#include "symbols.h"
#include <xasm.h>

u32 xasm_disassemble_bytes_uncolored(FILE* fp, xvm_bin* bin, const char* bytecode, u32 len, u32 address, u32 ninstr, u32 need_clean)
{
    return internal_xasm_disassemble_bytes(fp, bin, bytecode, len, address, ninstr, need_clean, 0);
}

u32 xasm_disassemble_bytes_colored(FILE* fp, xvm_bin* bin, const char* bytecode, u32 len, u32 address, u32 ninstr, u32 need_clean)
{
    return internal_xasm_disassemble_bytes(fp, bin, bytecode, len, address, ninstr, need_clean, 1);
}

u32 internal_xasm_disassemble_bytes(FILE* fp, xvm_bin* bin, const char* bytecode, u32 len, u32 address, u32 ninstr, u32 need_clean, u32 colored)
{
    // todo: check if an instruction that requires arguments
    //       has the argument modes set to NOARGS
    // need_clean : if you want only clean disassembly
    if (bytecode == NULL || len == 0) {
        for (u32 i = 0; i < ninstr; i++) {
            if (colored) {

                fprintf(fp, KBLU "0x%-14.8X" KNRM "Cannot access memory at address " KRED "0x%x" KNRM "\n", address + i, address + i);
            } else {

                fprintf(fp, "0x%-14.8XCannot access memory at address 0x%x\n", address + i, address + i);
            }
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
    u32 i = 0;
    for (i = 0; (pc < len) && i < ninstr; i++) {
        temp = resolve_symbol_name(bin->x_symtab, address + pc);
        if (temp != NULL) {
            if (colored) {
                printf(KBLU "\n0x%.8X" KNRM " <" KGRN "%s" KNRM ">:\n", address + pc, temp);
            } else {
                printf("\n0x%.8X <%s>:\n", address + pc, temp);
            }
        }
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
            if (colored) {
                fprintf(fp, "%-10.7s\n", KRED "(bad)" KNRM);
            } else {
                fprintf(fp, "%-10.7s\n", "(bad)");
            }
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
            if (colored) {
                fprintf(fp, "%s\n", KRED "(bad)" KNRM);
            } else {
                fprintf(fp, "%s\n", "(bad)");
            }
            continue;
        }

        if (mode2 != ARG_NARG && mode2 != ARG_REGD && mode2 != ARG_IMMD && !(mode2 & ARG_PTRD)) {
            clean_disassembly = E_ERR;
            if (need_clean != E_OK) {
                return E_ERR;
            }
            if (colored) {
                fprintf(fp, "%s\n", KRED "(bad)" KNRM);
            } else {
                fprintf(fp, "%s\n", "(bad)");
            }
            continue;
        }

        if (colored) {
            fprintf(fp, KBLU "0x%-14.8X" KNRM, address + pc - 2);
            fprintf(fp, KCYN "%-10.7s" KNRM, mnemonics[opcode]);
        } else {
            fprintf(fp, "0x%-14.8X", address + pc - 2);
            fprintf(fp, "%-10.7s", mnemonics[opcode]);
        }
        switch (mode1) {
        case ARG_NARG:
            break;
        case ARG_REGD: {
            reg = bytecode[pc++];
            if (reg >= sizeof(regid_2_str) / sizeof(regid_2_str[0])) {
                clean_disassembly = E_ERR;
                if (need_clean != E_OK) {
                    return E_ERR;
                }
                if (colored) {
                    fprintf(fp, "%s\n", KRED "(bad)" KNRM);
                } else {
                    fprintf(fp, "%s\n", "(bad)");
                }
                continue;
            }
            fprintf(fp, "%s", regid_2_str[reg]);
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
                    if (colored) {
                        fprintf(fp, KMAG "#0x%x" KNRM, -1 * imm);
                    } else {
                        fprintf(fp, "#0x%x", -1 * imm);
                    }
                } else {
                    if (colored) {
                        fprintf(fp, KMAG "#0x%x" KNRM, (signed int)imm);
                    } else {
                        fprintf(fp, "#0x%x", (signed int)imm);
                    }
                }
            } else {
                if (colored) {
                    fprintf(fp, KGRN "%s" KNRM, temp);
                } else {
                    fprintf(fp, "%s", temp);
                }
            }

            pc += sizeof(u32);
            break;
        }
        default: {
            if (mode1 & ARG_PTRD) {
                fprintf(fp, "[");
                if (mode1 & ARG_REGD) {
                    reg = bytecode[pc++];
                    if (reg >= sizeof(regid_2_str) / sizeof(regid_2_str[0])) {
                        clean_disassembly = E_ERR;
                        if (need_clean != E_OK) {
                            return E_ERR;
                        }
                        if (colored) {
                            fprintf(fp, "%s\n", KRED "(bad)" KNRM);
                        } else {
                            fprintf(fp, "%s\n", "(bad)");
                        }
                        continue;
                    }
                    fprintf(fp, "%s", regid_2_str[reg]);
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
                            if (colored) {
                                fprintf(fp, KMAG "#0x%x" KNRM, -1 * imm);
                            } else {
                                fprintf(fp, "#0x%x", -1 * imm);
                            }
                        } else {
                            if (colored) {
                                fprintf(fp, KMAG "#0x%x" KNRM, (signed int)imm);
                            } else {
                                fprintf(fp, "#0x%x", (signed int)imm);
                            }
                        }
                    } else {
                        if (colored) {
                            fprintf(fp, KGRN "%s" KNRM, temp);
                        } else {
                            fprintf(fp, "%s", temp);
                        }
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
            if (reg >= sizeof(regid_2_str) / sizeof(regid_2_str[0])) {
                clean_disassembly = E_ERR;
                if (need_clean != E_OK) {
                    return E_ERR;
                }
                if (colored) {
                    fprintf(fp, "%s\n", KRED "(bad)" KNRM);
                } else {
                    fprintf(fp, "%s\n", "(bad)");
                }
                continue;
            }
            fprintf(fp, "%s", regid_2_str[reg]);
            break;
        }
        case ARG_IMMD: {
            imm = *(u32*)&bytecode[pc];
            if (bin != NULL) {
                temp = resolve_symbol_name(bin->x_symtab, imm);
            }

            if (temp == NULL) {
                if (((signed int)imm) < 0) {
                    if (colored) {
                        fprintf(fp, KMAG "#0x%x" KNRM, -1 * imm);
                    } else {
                        fprintf(fp, "#0x%x", -1 * imm);
                    }
                } else {
                    if (colored) {
                        fprintf(fp, KMAG "#0x%x" KNRM, (signed int)imm);
                    } else {
                        fprintf(fp, "#0x%x", (signed int)imm);
                    }
                }
            } else {
                if (colored) {
                    fprintf(fp, KGRN "%s" KNRM, temp);
                } else {
                    fprintf(fp, "%s", temp);
                }
            }

            pc += sizeof(u32);
            break;
        }
        default: {
            if (mode2 & ARG_PTRD) {
                fprintf(fp, "[");
                if (mode2 & ARG_REGD) {
                    reg = bytecode[pc++];
                    if (reg >= sizeof(regid_2_str) / sizeof(regid_2_str[0])) {
                        clean_disassembly = E_ERR;
                        if (need_clean != E_OK) {
                            return E_ERR;
                        }
                        if (colored) {
                            fprintf(fp, "%s\n", KRED "(bad)" KNRM);
                        } else {
                            fprintf(fp, "%s\n", "(bad)");
                        }
                        continue;
                    }
                    fprintf(fp, "%s", regid_2_str[reg]);
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
                            if (colored) {
                                fprintf(fp, KMAG "#0x%x" KNRM, -1 * imm);
                            } else {
                                fprintf(fp, "#0x%x", -1 * imm);
                            }
                        } else {
                            if (colored) {
                                fprintf(fp, KMAG "#0x%x" KNRM, (signed int)imm);
                            } else {
                                fprintf(fp, "#0x%x", (signed int)imm);
                            }
                        }
                    } else {
                        if (colored) {
                            fprintf(fp, KGRN "%s" KNRM, temp);
                        } else {
                            fprintf(fp, "%s", temp);
                        }
                    }

                    pc += sizeof(u32);
                }
                fprintf(fp, "]");
                break;
            }
        }
        }
        fprintf(fp, KNRM "\n");
    }
    return i;
}
