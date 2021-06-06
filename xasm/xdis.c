//
// Created by X3eRo0 on 5/1/2021.
//

#include "../common/loader.h"
#include "xasm.h"

int main(int argc, char *argv[]){

    // initialize binary structure
    xvm_bin* bin = init_xvm_bin();
    section_entry* to_disasm = NULL;

    // argument checking
    if (argc < 2) {
        printf("Usage %s <xvm.bin>\n", argv[0]);
        fini_xvm_bin(bin); bin = NULL;
        return E_ERR;
    }

    xvm_bin_load_file(bin, argv[1]);

    if (argc == 3){
        // try to use argv[2] as address to disassemble
        u32 addr = xasm_resolve_number(argv[2]);
        to_disasm = find_section_entry_by_addr(bin->x_section, addr);
    } else{
        to_disasm = find_section_entry_by_name(bin->x_section, ".text");
    }

    show_exe_info(bin->x_header);
    show_section_info(bin->x_section);
    show_symtab_info(bin->x_symtab);
    xasm_disassemble(bin, to_disasm, 0);
    fini_xvm_bin(bin); bin = NULL;

    return E_OK;
}