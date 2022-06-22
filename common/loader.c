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


u32 xvm_bin_read_exe_header(xvm_bin* bin){

    if (!bin->x_header) {
        return E_ERR;
    }

    fseek(bin->x_file, 0, SEEK_SET); // go to the beginning
    fread(&bin->x_header->x_magic,    1, sizeof(u32), bin->x_file);
    fread(&bin->x_header->x_entry,    1, sizeof(u32), bin->x_file);
    fread(&bin->x_header->x_dbgsym,   1, sizeof(u32), bin->x_file);
    fread(&bin->x_header->x_szfile,   1, sizeof(u32), bin->x_file);
    fread(&bin->x_header->x_sections, 1, sizeof(u32), bin->x_file);

    // FIXME: add security checks
    if (bin->x_header->x_magic != XVM_MAGIC){
        fprintf(stderr, "[" KRED "-" KNRM "] Corrupted Bytecode\n");
        exit(E_ERR);
    }

    return E_OK;
}



u32 xvm_bin_show_exe_info(xvm_bin* bin){

    if (!bin) {
        return E_ERR;
    }
    show_exe_info(bin->x_header);
    return E_OK;
}

u32 show_exe_info(exe_header* header){

    if (!header) {
        return E_ERR;
    }
    // FIXME : COMMENT FOR RELEASE

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
    bin->x_header     = init_exe_header();
    bin->x_symtab     = init_symtab();
    bin->x_section    = init_section();
    bin->x_file       = NULL;

    return bin;
}


u32 xvm_bin_load_file(xvm_bin* bin, char* filename){

    if (!bin){
        return E_ERR;
    }

    if (xvm_bin_open_file(bin, filename) == E_ERR){
        fprintf(stderr, "[" KRED "-" KNRM "] Cannot open \"%s\"\n", filename);
        exit(-1);
    }
    xvm_bin_read_exe_header(bin);

    struct raw_symtab_t {
        u32 offset;
        u32 address;
    } * raw_symtab = (struct raw_symtab_t*) malloc(sizeof(struct raw_symtab_t) * bin->x_header->x_dbgsym);

    // read raw symtab with offset instead os the string
    for (u32 i = 0; i < bin->x_header->x_dbgsym; i++){
        fread(&raw_symtab[i].offset, sizeof(u32), 1, bin->x_file);
        fread(&raw_symtab[i].address, sizeof(u32), 1, bin->x_file);
    }

    // read sections
    section_entry* section = NULL;
    char*   section_name = NULL;
    size_t  read_size    = 0;
    u32     section_size = 0;
    u32     section_flag = 0;
    u32     section_indx = 0;
    u32     section_addr = 0;
    u32     section_head = 0;

    for (u32 i = 0; i < bin->x_header->x_sections; i++) {

        // read section name
        fread(&section_head, sizeof(u32), 1, bin->x_file);
        if (section_head != 0xDEADBEEF){
            fprintf(stderr, "[" KRED "-" KNRM "] Corrupted Section Headers\n");
            exit(1);
        }

        if (getdelim(&section_name, &read_size, '\x00', bin->x_file) <= 0) {
            fprintf(stderr, "[" KRED "-" KNRM "] Corrupted Section Headers\n");
            exit(1);
        }

        fread(&section_size, sizeof(u32), 1, bin->x_file);
        fread(&section_addr, sizeof(u32), 1, bin->x_file);
        fread(&section_flag, sizeof(u32), 1, bin->x_file);
        fread(&section_indx, sizeof(u32), 1, bin->x_file);

        // if size is greater than the limit, adjust the size
        section_size = section_size > MAX_ALLOC_SIZE ? MAX_ALLOC_SIZE : section_size;
        section_indx = section_indx > MAX_ALLOC_SIZE ? MAX_ALLOC_SIZE : section_indx;
        section_indx = section_indx > section_size ? section_size : section_indx;

        section = add_section(bin->x_section, section_name, section_size, section_addr, section_flag);

        if (section == NULL){
            fprintf(stderr, "[" KRED "-" KNRM "] Cannot Load Section (\"%s\") : FATAL @ 0x%x\n", section_name, section_addr);
            exit(-1);
        }

        // after allocating the section read and fill the bytes
        for (u32 j = 0, byte = 0; j < section_indx && (byte = fgetc(bin->x_file)) != -1; j++){
            append_byte(bin->x_section, section, byte);
        }
    }

    free(section_name); section_name = NULL;

    // after reading bytes rebuild the symtab
    // by using strings in .data section

    section = find_section_entry_by_name(bin->x_section, ".data");

    if (section == NULL){
        fprintf(stderr, "[" KRED "-" KNRM "] Corrupted section headers: \".data\" Not Found\n");
        exit(-1);
    }

    for (u32 j = 0; j < bin->x_header->x_dbgsym; j++){
        if (raw_symtab[j].offset > section->v_size){
            fprintf(stderr, "[" KRED "-" KNRM "] Corrupted section headers: symbol offset (0x%x) is out of bounds for \".data\" section\n", raw_symtab[j].offset);
            exit(-1);
        }
        add_symbol(bin->x_symtab, &section->m_buff[raw_symtab[j].offset], raw_symtab[j].address);
    }

    free(raw_symtab); raw_symtab = NULL;
    return E_OK;
}

u32 xvm_bin_open_file(xvm_bin* bin, char* filename){

    if (!bin){
        return E_ERR;
    }

    if (bin->x_file){
        xvm_bin_close_file(bin);
    }

    bin->x_file = fopen(filename, "r");

    if (!bin->x_file){
        return E_ERR;
    }

    return E_OK;
}

u32 xvm_bin_close_file(xvm_bin* bin){

    if (!bin){
        return E_ERR;
    }

    if (bin->x_file){
        fclose(bin->x_file); bin->x_file = NULL;
    }

    return E_OK;
}


u32 fini_xvm_bin(xvm_bin* bin) {
    // destroy binary structure
    fini_exe_header(bin->x_header);
    fini_symtab(bin->x_symtab);
    fini_section(bin->x_section);
    xvm_bin_close_file(bin);

    memset(bin, 0, sizeof(xvm_bin));
    free(bin); bin = NULL;
    return E_OK;
}
