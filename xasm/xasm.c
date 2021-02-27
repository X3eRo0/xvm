//
// Created by X3eRo0 on 2/23/2021.
//

#include "xasm.h"


int main(int argc, char* argv[]){


    xasm* xasm = init_xasm();

    if (argc != 3) {
        printf("Usage %s <input.asm> <output.bin>\n", argv[0]);
        return E_ERR;
    }

    open_ifile(xasm, argv[1]);
    open_ofile(xasm, argv[2]);
    xasm_assemble(xasm);
    fini_xasm(xasm);
    xasm = NULL;
    return E_OK;

}