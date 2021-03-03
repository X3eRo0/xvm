//
// Created by X3eRo0 on 2/21/2021.
//

#ifndef XVM_LOADER_H
#define XVM_LOADER_H

#include "../common/const.h"
#include "../common/symbols.h"
#include "../common/sections.h"

typedef struct exe_header_t {
    // xvm binary header

    u32 x_magic;    // xvm magic header
    u32 x_entry;    // xvm entry point
    u32 x_dbgsym;   // number of debug symbols if enabled
    u32 x_szfile;   // size of file in bytes
    u32 x_sections; // no of sections in file

} exe_header;

typedef struct xvm_bin_t {
    // xvm binary structure

    exe_header* x_header;     // xvm header
    symtab*     x_symtab;     // xvm symbols if any
    section*    x_section;    // xvm sections
    FILE*       x_file;       // xvm file

} xvm_bin;

xvm_bin*    init_xvm_bin();
u32         xvm_bin_set_ofile(xvm_bin* bin, FILE* ofile);
u32         xvm_bin_set_ifile(xvm_bin* bin, FILE* ifile);
u32         xvm_bin_read_exe_header(xvm_bin* bin);
u32         xvm_bin_show_exe_info(xvm_bin* bin);
u32         xvm_bin_load_file(xvm_bin* bin, char* filename);
u32         xvm_bin_open_file(xvm_bin* bin, char* filename);
u32         xvm_bin_close_file(xvm_bin* bin);
u32         fini_xvm_bin(xvm_bin* bin);
exe_header* init_exe_header();
u32         show_exe_info(exe_header * bin);
u32         write_exe_header(exe_header* header, FILE* ofile);
u32         display_exe_header(exe_header* header);
u32         fini_exe_header(exe_header* header);

#endif //XVM_LOADER_H
