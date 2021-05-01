//
// Created by X3eRo0 on 5/1/2021.
//

#include "xasm.h"


const char* mnemonics[XVM_NINSTR] = {
        [XVM_OP_MOV]  = "mov",
        [XVM_OP_MOVB] = "movb",
        [XVM_OP_MOVW] = "movw",
        [XVM_OP_NOP]  = "nop",
        [XVM_OP_HLT]  = "hlt",
        [XVM_OP_RET]  = "ret",
        [XVM_OP_CALL] = "call",
        [XVM_OP_SYSC] = "syscall",
        [XVM_OP_ADD]  = "add",
        [XVM_OP_SUB]  = "sub",
        [XVM_OP_MUL]  = "mul",
        [XVM_OP_DIV]  = "div",
        [XVM_OP_XOR]  = "xor",
        [XVM_OP_AND]  = "and",
        [XVM_OP_OR]   = "or",
        [XVM_OP_NOT]  = "not",
        [XVM_OP_PUSH] = "push",
        [XVM_OP_POP]  = "pop",
        [XVM_OP_XCHG] = "xchg",
        [XVM_OP_INC]  = "inc",
        [XVM_OP_DEC]  = "dec",
        [XVM_OP_CMP]  = "cmp",
        [XVM_OP_TEST] = "test",
        [XVM_OP_JMP]  = "jmp",
        [XVM_OP_JZ]   = "jz",
        [XVM_OP_JNZ]  = "jnz",
        [XVM_OP_JA]   = "ja",
        [XVM_OP_JB]   = "jb",
};

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


void xasm_disassemble(xvm_bin* bin, u32 ninstr){

    section_entry* text = find_section_entry_by_name(bin->x_section, ".text");
    printf("\n[" KGRN "+" KNRM "] Disassembling .text\n");
    printf("[" KGRN "+" KNRM "] Raw Size : %d BYTES\n", text->m_ofst);
    printf("[" KGRN "+" KNRM "] Address  : 0x%X\n", text->v_addr);
    char* bytecode = text->m_buff;    // bytecode array
    u32 pc = 0;                       // pc

    u8 opcd = 0;
    u8 mode = 0;
    u8 mode1 = 0;
    u8 mode2 = 0;
    u32 imm = 0;
    char *temp = NULL;

    if (ninstr == 0){
        ninstr = -1;
    }

    for (u32 i = 0; (pc < text->m_ofst - 4) && i < ninstr; i++){

        imm = text->v_addr + pc;
        temp = resolve_symbol_name(bin->x_symtab, imm);

        if (temp != NULL){
            printf(KBLU "\n0x%.8X" KNRM " <" KGRN "%s" KNRM ">:\n", imm, temp);
        }

        printf(KBLU "0x%-14.8X" KNRM, imm);

        opcd = bytecode[pc++];
        mode = bytecode[pc++];

        mode1 = (mode >> 0x0) & 0xf;
        mode2 = (mode >> 0x4) & 0xf;

        printf(KCYN "%-10.7s" KNRM, mnemonics[opcd]);

        /* ARG1 */
        switch(mode1){
            case ARG_NARG: break;
            case ARG_REGD: {
                printf("%s", registers[bytecode[pc++]]);
                break;
            }
            case ARG_IMMD: {
                imm = *(u32 *)&bytecode[pc];
                temp = resolve_symbol_name(bin->x_symtab, imm);

                if (temp == NULL){
                    printf(KMAG "#0x%x" KNRM, imm);
                } else {
                    printf(KGRN "%s" KNRM, temp);
                }

                pc += sizeof(u32);
                break;
            }
            default:{
                if (mode1 & ARG_PTRD){
                    printf("[");

                    if (mode1 & ARG_REGD){
                        printf("%s", registers[bytecode[pc++]]);
                    }

                    if (mode1 == (ARG_REGD | ARG_IMMD)){
                        printf(" + ");
                    }

                    if (mode1 & ARG_IMMD){
                        imm = *(u32 *)&bytecode[pc];
                        temp = resolve_symbol_name(bin->x_symtab, imm);

                        if (temp == NULL){
                            printf(KMAG "#0x%x" KNRM, imm);
                        } else {
                            printf(KGRN "%s" KNRM, temp);
                        }
                        pc += sizeof(u32);
                    }
                    printf("]");
                }
            }
        }

        if (mode2 != ARG_NARG){
            printf(", ");
        }

        /* ARG2 */
        switch(mode2){
            case ARG_NARG: break;
            case ARG_REGD: {
                printf("%s", registers[bytecode[pc++]]);
                break;
            }
            case ARG_IMMD: {
                imm = *(u32 *)&bytecode[pc];
                temp = resolve_symbol_name(bin->x_symtab, imm);

                if (temp == NULL){
                    printf(KMAG "#0x%x" KNRM, imm);
                } else {
                    printf(KGRN "%s" KNRM, temp);
                }
                pc += sizeof(u32);
                break;
            }
            default:{
                if (mode2 & ARG_PTRD){
                    printf("[");

                    if (mode2 & ARG_REGD){
                        printf("%s", registers[bytecode[pc++]]);
                    }

                    if (mode2 == (ARG_REGD | ARG_IMMD)){
                        printf(" + ");
                    }

                    if (mode2 & ARG_IMMD){
                        imm = *(u32 *)&bytecode[pc];
                        temp = resolve_symbol_name(bin->x_symtab, imm);

                        if (temp == NULL){
                            printf(KMAG "#0x%x" KNRM, imm);
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