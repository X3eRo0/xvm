//
// Created by X3eRo0 on 5/1/2021.
//

#include "../common/loader.h"
#include "xasm.h"

int main(int argc, char *argv[]){

    // initialize binary structure
    xvm_bin* bin = init_xvm_bin();

    // argument checking
    if (argc != 2) {
        printf("Usage %s <xvm.bin>\n", argv[0]);
        fini_xvm_bin(bin); bin = NULL;
        return E_ERR;
    }

    xvm_bin_load_file(bin, argv[1]);
    show_exe_info(bin->x_header);
    show_section_info(bin->x_section);
    show_symtab_info(bin->x_symtab);
    xasm_disassemble(bin, 0);
    fini_xvm_bin(bin); bin = NULL;

    return E_OK;
}