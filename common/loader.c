//
// Created by X3eRo0 on 2/21/2021.
//

#include "loader.h"

exe_header* init_exe_header(){
    // create header structure
    exe_header* header  = (exe_header*) malloc(sizeof(exe_header));
    header->x_magic     = XVM_MAGIC;
    header->x_entry     = XVM_DFLT_EP;
    header->x_dbgsym    = 0;
    header->x_sections  = 0;
    header->x_szfile    = 0;

    return header;
}

u32 write_exe_header(exe_header* header, FILE* ofile){

    if (!header || !ofile) {
        return E_ERR;
    }

    fseek(ofile, 0, SEEK_SET); // go to the beginning
    fwrite(&header->x_magic,    1, sizeof(u32), ofile);
    fwrite(&header->x_entry,    1, sizeof(u32), ofile);
    fwrite(&header->x_dbgsym,   1, sizeof(u32), ofile);
    fwrite(&header->x_szfile,   1, sizeof(u32), ofile);
    fwrite(&header->x_sections, 1, sizeof(u32), ofile);

    return E_OK;
}

u32 xvm_bin_read_exe_header(xvm_bin* bin, FILE* ifile){

    if (!bin->header) {
        return E_ERR;
    }

    fseek(ifile, 0, SEEK_SET); // go to the beginning
    fread(&bin->header->x_magic,    1, sizeof(u32), ifile);
    fread(&bin->header->x_entry,    1, sizeof(u32), ifile);
    fread(&bin->header->x_dbgsym,   1, sizeof(u32), ifile);
    fread(&bin->header->x_szfile,   1, sizeof(u32), ifile);
    fread(&bin->header->x_sections, 1, sizeof(u32), ifile);

    return E_OK;
}

u32 xvm_bin_show_exe_info(xvm_bin* bin){

    if (!bin) {
        return E_ERR;
    }
    show_exe_info(bin->header);
    return E_OK;
}

u32 show_exe_info(exe_header* header){

    if (!header) {
        return E_ERR;
    }

    puts(KBLU "\n[-----------------" KNRM "xvm" KBLU "-----------------]" KNRM);
    printf("[\033[0;32m+\033[0m] x_magic\t\t: %.8X\n", header->x_magic);
    printf("[\033[0;32m+\033[0m] x_szfile\t\t: %d BYTES\n", header->x_szfile);
    printf("[\033[0;32m+\033[0m] x_entry\t\t: 0x%.8x\n", header->x_entry);

    if (header->x_dbgsym > 0){
        printf("[\033[0;32m+\033[0m] x_dbgsym\t\t: 0x%08x\n", header->x_dbgsym);
    } else {
        printf("[\033[0;32m+\033[0m] x_dbgsym\t\t: False\n");
    }
    puts(KBLU "[-------------------------------------]\n" KNRM);
    return E_OK;

}

u32 fini_exe_header(exe_header* header){
    // destroy exe header
    memset(header, 0, sizeof(exe_header));
    free(header); header = NULL;
    return E_OK;
}

xvm_bin* init_xvm_bin(){
    // create binary structure
    xvm_bin* bin = (xvm_bin*) malloc(sizeof(xvm_bin));
    bin->header     = init_exe_header();
    bin->symtab     = NULL; // must be set by the assembler later

    return bin;
}


u32 fini_xvm_bin(xvm_bin* bin) {
    // destroy binary structure
    fini_exe_header(bin->header);
    memset(bin, 0, sizeof(xvm_bin));
    free(bin); bin = NULL;
    return E_OK;
}