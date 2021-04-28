//
// Created by X3eRo0 on 2/23/2021.
//

#include "xasm.h"

static u32 ifiles = 0;
static FILE** inputf = NULL;
static char*  output = NULL;
static u32 dbgsyms = 0;

int main(int argc, char* argv[]){

    // initialize assembler structure
    xasm*   xasm = init_xasm();

    for (u32 i = 0; i < argc; i++){
        // if -i count until we reach other flags
        if (argv[i][0] == '-' && argv[i][1] == 'i'){
            u32 j = ++i;
            while (argv[j] != NULL && argv[j++][0] != '-') {
                ifiles++;
                inputf = realloc(inputf, sizeof(char *) * ifiles);
                inputf[ifiles - 1] = fopen(argv[i++], "r");
            }
        }
        if (argv[i][0] == '-' && argv[i][1] == 'o'){
            output = argv[++i];
        }
        if (argv[i][0] == '-' && argv[i][1] == 'd'){
            dbgsyms = 1;
        }
        if (!strncmp(argv[i], "--help", 6)){
            puts("Usage: ./xasm -i <input files> -o <output file>");
            puts("Author: X3eRo0");
            puts("\t-i -- \tInput Files");
            puts("\t-o -- \tOutput File");
            puts("\t-h -- \tHelp");
            exit(-1);
        }
    }


    // add default .text and .data sections
    xasm->sections = init_section();
    section_entry* text = add_section(xasm->sections, ".text", 0x1000, XVM_DFLT_EP, PERM_READ | PERM_EXEC);
    section_entry* data = add_section(xasm->sections, ".data", 0x1000, XVM_DFLT_DP, PERM_READ | PERM_WRITE);

    // open the input and output files
    // for assembling

    xasm_open_ofile(xasm, output);

    // assemble loop
    xasm_assemble(xasm, text, inputf, ifiles);

    // xvm binary headers
    xasm->header->x_szfile = get_total_size(xasm);
    xasm->header->x_dbgsym = xasm->symtab->n_symbols;
    xasm->header->x_sections = xasm->sections->n_sections;
    xasm->header->x_entry = resolve_symbol_addr(xasm->symtab, "_start");

    // write headers
    xasm->header->x_dbgsym *= dbgsyms;
    write_exe_header(xasm->header, xasm->ofile);

    // show headers
    show_exe_info(xasm->header);

    // write symbol table
    if (dbgsyms) {
        write_symtab_to_file(xasm->symtab, xasm->sections, xasm->ofile);
    }

    // write section data to files
    write_section_to_file(xasm->sections, xasm->ofile);

    // destroy assembler structure
    fini_xasm(xasm); xasm = NULL;

    for (u32 i = 0; i < ifiles; i++){
        fclose(inputf[i]); inputf[i] = NULL;
    }

    free(inputf); inputf = NULL;

    // return successfully
    return E_OK;
}