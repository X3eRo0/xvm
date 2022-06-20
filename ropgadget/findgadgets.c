#include "ropgadget.h"

void xvm_find_ropgadgets(xvm_bin* bin, section_entry* sec, u32 depth){
    // make sure section is executable

    if (sec->m_name == NULL){
        printf("\n[" KGRN "+" KNRM "] Disassembling <Unnamed Section>\n");
    } else {
        printf("\n[" KGRN "+" KNRM "] Disassembling %s\n", sec->m_name);
    }
    printf("[" KGRN "+" KNRM "] Raw Size : %d BYTES\n", sec->m_ofst);
    printf("[" KGRN "+" KNRM "] Address  : 0x%X\n", sec->v_addr);

    /*
     * How to find ropgadgets for xvm?
     * A step by step guide: 
     * 1. First step is pretty simple, Just find all the return opcodes
     *    which can be done by doing a linear search for return opcode.
     *
     * 2. XVM has multiple length instructions. So, we have to scan the
     *    memory for valid instructions before the return opcode.
     *
     * 3. XVM has instructions of length 2, 4, and 6. This makes our job
     *    easier as we just have to find instructions which are of the
     *    above lengths instead of all ranges (for ex. 1, 2, 3, 4 BYTES
     *    before the return opcode.)
     *
     */

    u8  tbyte = 0;

    for (int _counter = 0; _counter < sec->v_size; _counter++){
        tbyte = (u8) sec->m_buff[_counter];
        if (tbyte == XVM_OP_RET){ // .db 0x05
            // find all possible gadgets in the specified depth
        }
    }

}
