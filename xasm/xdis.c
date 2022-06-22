//
// Created by X3eRo0 on 5/1/2021.
//

#include <loader.h>
#include <xasm.h>
#include <stdlib.h>

int main(int argc, char *argv[]){

    // initialize binary structure
    xvm_bin* bin = init_xvm_bin();
    section_entry* to_disasm = NULL;
    u32 addr = 0;
    u32 nins = 0;

    // argument checking
    if (argc < 2) {
        printf("Usage %s <xvm.bin>\n", argv[0]);
        fini_xvm_bin(bin); bin = NULL;
        return E_ERR;
    }

    if (!strncmp(argv[1], "--help", 6)){
        printf("Usage %s <xvm.bin> <address in hex to disassemble> <number of instructions to disassemble>\n", argv[0]);
        return E_OK;
    }

    xvm_bin_load_file(bin, argv[1]);
    if (argc >= 3){
        // try to use argv[2] as address to disassemble
        nins = xasm_resolve_number(argv[3]); // if argc == 3 argv[3] will be NULL and nins will be 0
        addr = xasm_resolve_number(argv[2]);
        to_disasm = find_section_entry_by_addr(bin->x_section, addr);
    } else{
        to_disasm = find_section_entry_by_name(bin->x_section, ".text");
        addr = to_disasm->v_addr;
    }

    show_exe_info(bin->x_header);
    show_section_info(bin->x_section);
    show_symtab_info(bin->x_symtab);
    xasm_disassemble(bin, to_disasm, addr, 0);
    fini_xvm_bin(bin); bin = NULL;

    return E_OK;
}
