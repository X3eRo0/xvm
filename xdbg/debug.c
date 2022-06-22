#include <xdbg.h>

void handle_signals(xvm_cpu* cpu, xvm_bin* bin){
    // handle_cpu signals
    if (cpu->errors->signal_id == XSIGTRAP){
        // add breakpoint
        printf("TRAP\n");
    }
}

void dbg_cpu(xvm_cpu* cpu, xvm_bin* bin)
{
    u32 instr_size = 0;
    while (get_RF(cpu)) {
        instr_size = do_execute(cpu, bin);
        handle_signals(cpu, bin);
    }
}
