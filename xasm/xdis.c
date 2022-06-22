//
// Created by X3eRo0 on 5/1/2021.
//

#include <loader.h>
#include <stdio.h>
#include <stdlib.h>
#include <xasm.h>

int main(int argc, char* argv[])
{

    // initialize binary structure
    xvm_bin* bin = init_xvm_bin();
    section_entry* to_disasm = NULL;
    u32 addr = 0;
    u32 nins = 0;

    // argument checking
    if (argc < 2) {
        fprintf(stderr, "Usage %s <xvm.bin>\n", argv[0]);
        fini_xvm_bin(bin);
        bin = NULL;
        return E_ERR;
    }

    if (!strncmp(argv[1], "--help", 6)) {
        fprintf(stderr, "Usage %s <xvm.bin> <address in hex to disassemble> <number of instructions to disassemble>\n", argv[0]);
        return E_OK;
    }

    xvm_bin_load_file(bin, argv[1]);
    if (argc >= 3) {
        // try to use argv[2] as address to disassemble
        nins = xasm_resolve_number(argv[3]); // if argc == 3 argv[3] will be NULL and nins will be 0
        addr = xasm_resolve_number(argv[2]);
        to_disasm = find_section_entry_by_addr(bin->x_section, addr);
        if (to_disasm == NULL) {
            fprintf(stderr, "[!] Error: 0x%.8X address is not mapped\n", addr);
            return E_ERR;
        }
    } else {
        to_disasm = find_section_entry_by_name(bin->x_section, ".text");
        if (to_disasm == NULL) {
            fprintf(stderr, "[!] Error: could not found .text section (possible corrupt xvm file)\n");
            return E_ERR;
        }
        addr = to_disasm->v_addr;
    }

    show_exe_info(bin->x_header);
    show_section_info(bin->x_section);
    show_symtab_info(bin->x_symtab);
    xasm_disassemble_bytes_colored(stdout, bin, to_disasm->m_buff, to_disasm->v_size, addr, 0, 1);
    fini_xvm_bin(bin);
    bin = NULL;

    return E_OK;
}
