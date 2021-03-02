//
// Created by X3eRo0 on 2/23/2021.
//

#include "xasm.h"

int main(int argc, char* argv[]){


    xasm*   xasm = init_xasm();

    if (argc != 3) {
        printf("Usage %s <input.asm> <output.bin>\n", argv[0]);
        return E_ERR;
    }

    xasm->sections = init_section();
    add_section(xasm->sections, ".text", 0x1000, PERM_READ | PERM_EXEC);
    add_section(xasm->sections, ".data", 0x1000, PERM_READ | PERM_WRITE);
    open_ifile(xasm, argv[1]);
    open_ofile(xasm, argv[2]);


    xasm_assemble(xasm);
    xasm->header->x_szfile = get_total_size(xasm);
    xasm->header->x_dbgsym = xasm->symtab->n_symbols;
    xasm->header->x_sections = xasm->sections->n_sections;
    write_exe_header(xasm->header, xasm->ofile);
    show_exe_info(xasm->header);
    write_symtab_to_file(xasm->symtab, xasm->sections, xasm->ofile);
    write_section_to_file(xasm->sections, xasm->ofile);
    fini_xasm(xasm);
    xasm = NULL;
    return E_OK;

}