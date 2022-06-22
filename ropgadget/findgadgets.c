#include "xasm.h"
#include <ropgadget.h>
#include <stdio.h>

void xvm_find_ropgadgets(xvm_bin* bin, section_entry* sec, u32 depth)
{
    // make sure section is executable

    if (sec->m_name == NULL) {
        xasm_info("Disassembling <Unnamed Section>\n");
    } else {
        xasm_info("Disassembling %s\n", sec->m_name);
    }
    xasm_info("Raw Size : %d BYTES\n", sec->m_ofst);
    xasm_info("Address  : 0x%X\n", sec->v_addr);

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

    u8 tbyte = 0;
    FILE* fp = fopen("/dev/null", "w");
    u32 ninstr = 0;
    u32 dislen = 0;

    if (fp == NULL) {
        xasm_warn("could not get a handle on /dev/null\n");
        exit(0);
    }
    for (int _counter = 0; _counter < sec->v_size; _counter++) {
        tbyte = (u8)sec->m_buff[_counter];
        if (tbyte == XVM_OP_RET) { // .db 0x05
            // find all possible gadgets in the specified depth
            // First we find 2 byte instructions
            // depth is number of instructions not number of bytes
            // _i counts number of bytes
            u32 _i = 2; // start from &ret_instr - 2
            while (_i < 4 * depth) { // 4 works the best
                // do checks
                // 1. There must not be multiple ret instructions in the gadget printed. (check for ret instructions)
                // 2. gadget must end at ret instruction
                // 3. gadget must not start from a jmp instruction
                u8* fixed_ptr = (u8*)sec->m_buff + _counter - _i; // ptr to contain fixed address after doing checks;
                for (u8* _j = (u8*)sec->m_buff + _counter - _i; _j < (u8*)(sec->m_buff + _counter - 2); _j++) {
                    // skip the ret instruction
                    if (*_j == XVM_OP_RET || *_j == XVM_OP_HLT) {
                        fixed_ptr = _j + 2;
                    }

                    // must not start from a jmp
                    if (*_j >= XVM_OP_JMP && *_j <= XVM_OP_RJBE) {
                        fixed_ptr = _j + 7;
                    }
                }
                dislen = ((u8*)(sec->m_buff + _counter) - fixed_ptr) + 2;
                u32 address = sec->v_addr + (fixed_ptr - (u8*)sec->m_buff);
                // check if disassembled instruction is valid or not
                if ((ninstr = xasm_disassemble_bytes_uncolored(fp, bin, (char*)fixed_ptr, dislen, 0, 0, 1)) != E_ERR) {
                    /* printf("==============START=============\naddress: 0x%x\nlen: 0x%x\nninstrs: 0x%x\n", address + 2, dislen, depth); */
                    xasm_info("Gadget : 0x%x\n", address);
                    xasm_disassemble_bytes_uncolored(stdout, bin, (char*)fixed_ptr, dislen, address, 0, 1);
                }
                _i += 1;
            }
        }
    }

    fclose(fp);
    /* printf("ret: %x\n", xasm_disassemble_bytes(stdout, bin, sec->m_buff, 0x10, 0, 10, 1)); */
}
