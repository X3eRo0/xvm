//
// Created by X3eRo0 on 2/23/2021.
//

#include "xasm.h"

int main(int argc, char* argv[]){

    // initialize assembler structure
    xasm*   xasm = init_xasm();

    // argument checking
    if (argc != 3) {
        printf("Usage %s <input.asm> <output.bin>\n", argv[0]);
        return E_ERR;
    }

    // add default .text and .data sections
    xasm->sections = init_section();
    section_entry* text = add_section(xasm->sections, ".text", 0x1000, PERM_READ | PERM_EXEC);
    section_entry* data = add_section(xasm->sections, ".data", 0x1000, PERM_READ | PERM_WRITE);

    // open the input and output files
    // for assembling

    xasm_open_ifile(xasm, argv[1]);
    xasm_open_ofile(xasm, argv[2]);

    // assemble loop
    xasm_assemble(xasm, text);

    // xvm binary headers
    xasm->header->x_szfile = get_total_size(xasm);
    xasm->header->x_dbgsym = xasm->symtab->n_symbols;
    xasm->header->x_sections = xasm->sections->n_sections;

    // write headers
    write_exe_header(xasm->header, xasm->ofile);

    // show headers
    show_exe_info(xasm->header);

    // write symbol table
    write_symtab_to_file(xasm->symtab, xasm->sections, xasm->ofile);

    // write section data to files
    write_section_to_file(xasm->sections, xasm->ofile);

    // destroy assembler structure
    fini_xasm(xasm); xasm = NULL;

    // return successfully
    return E_OK;
}