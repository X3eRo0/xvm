#include <iface.h>
#include <xdbg.h>

int main(int argc, char* argv[])
{

    if (argc != 2) {
        fprintf(stderr, "Usage: xdbg <bytecode>\n");
        exit(-1);
    }

    setbuf(stdin, 0);
    setbuf(stdout, 0);

    xvm_cpu* cpu = init_xvm_cpu();
    xvm_bin* bin = init_xvm_bin();

    xvm_bin_load_file(bin, argv[1]);
    show_exe_info(bin->x_header);
    show_section_info(bin->x_section);
    show_symtab_info(bin->x_symtab);

    add_section(bin->x_section, "stack", XVM_STACK_SIZE, XVM_DFLT_SP & 0xfffff000, PERM_READ | PERM_WRITE);

    cpu->regs.pc = bin->x_header->x_entry; // set pc to entry point
    cpu->regs.sp = XVM_DFLT_SP;

    /* dbg_cpu(cpu, bin); */
    xdbg_iface(cpu, bin);

    fini_xvm_cpu(cpu);
    cpu = NULL;
    fini_xvm_bin(bin);
    bin = NULL;

    return E_OK;
}
