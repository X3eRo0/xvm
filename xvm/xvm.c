#include "cpu.h"

int main(int argc, char* argv[]) {

    // FIXME: Bug still remains. adding a check to prevent unmapping of head section also prevents
    // it from unmapping in the destruction function.

	if (argc != 2){
        fprintf(stderr, "Usage: xvm <bytecode>\n");
        exit(-1);
	}

	xvm_cpu * cpu = init_xvm_cpu();
	xvm_bin * bin = init_xvm_bin();
    xvm_bin_load_file(bin, argv[1]);
    show_exe_info(bin->x_header);
    show_section_info(bin->x_section);
    show_symtab_info(bin->x_symtab);

    add_section(bin->x_section, "stack", XVM_STACK_SIZE, XVM_DFLT_SP, PERM_READ | PERM_WRITE);

    cpu->regs.pc = bin->x_header->x_entry; // set pc to entry point
    cpu->regs.sp = XVM_DFLT_SP;

    fde_cpu(cpu, bin);

    fini_xvm_cpu(cpu); cpu = NULL;
    fini_xvm_bin(bin); bin = NULL;

    return E_OK;
}