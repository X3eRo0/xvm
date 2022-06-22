//
// Created by X3eRo0 on 6/20/2022.
//

#include <ropgadget.h>
#include <stdio.h>
#include <stdlib.h>

int main(int argc, char* argv[])
{

    // initialize binary structure
    xvm_bin* bin = init_xvm_bin();
    section_entry* section = NULL;
    u32 addr = 0;
    u32 nins = 0;

    // argument checking
    if (argc < 2) {
        printf("Usage %s <xvm.bin>\n", argv[0]);
        fini_xvm_bin(bin);
        bin = NULL;
        return E_ERR;
    }

    if (!strncmp(argv[1], "--help", 6)) {
        printf("Usage %s <xvm.bin> <address of section>\n", argv[0]);
        return E_OK;
    }

    xvm_bin_load_file(bin, argv[1]);
    if (argc == 3) {
        // try to use argv[2] as address to disassemble
        addr = xasm_resolve_number(argv[2]);
        section = find_section_entry_by_addr(bin->x_section, addr);
        if (section == NULL) {
            fprintf(stderr, "[!] Error: 0x%.8X address is not mapped\n", addr);
            return E_ERR;
        }
    } else {
        section = find_section_entry_by_name(bin->x_section, ".text");
        if (section == NULL) {
            fprintf(stderr, "[!] Error: could not found .text section (possible corrupt xvm file)\n");
            return E_ERR;
        }
        addr = section->v_addr;
    }

    // find rop gadgets in section
    show_exe_info(bin->x_header);
    show_section_info(bin->x_section);
    show_symtab_info(bin->x_symtab);
    xvm_find_ropgadgets(bin, section, 4);
    fini_xvm_bin(bin);
    bin = NULL;

    return E_OK;
}
