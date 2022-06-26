#include <commands.h>
#include <xdbg.h>

void handle_signals(iface_state* state)
{
    if (state == NULL) {
        xdbg_error("Something went wrong.\n");
        return;
    }
    if (state->cpu == NULL || state->bin == NULL) {
        xdbg_error("Something went wrong.\n");
        return;
    }

    // handle_cpu signals
    u32 signalid = NOSIGNAL;
    if (state->cpu->errors->signal_id != NOSIGNAL) {
        signalid = state->cpu->errors->signal_id;
        state->cpu->errors->signal_id = NOSIGNAL;
        state->cpu->errors->error_addr = 0;
    }
    if (state->bin->x_section->errors->signal_id != NOSIGNAL) {
        signalid = state->bin->x_section->errors->signal_id;
        state->bin->x_section->errors->signal_id = NOSIGNAL;
        state->bin->x_section->errors->error_addr = 0;
    }

    if (signalid == NOSIGNAL) {
        return;
    }
    // fix cpu state
    state->cpu->regs.pc -= 2;
    set_RF(state->cpu, 0);
    switch (signalid) {
    case XSIGSEGV: {
        xdbg_info(
            "Segmentation fault at #0x%.8X\n", state->cpu->regs.pc);
        break;
    }
    case XSIGTRAP: {
        xdbg_info(
            "Break point hit at #0x%.8X\n", state->cpu->regs.pc);
        break;
    }
    case XSIGSTOP: {
        xdbg_info(
            "Received SIGSTOP at #0x%.8X\n", state->cpu->regs.pc);
        break;
    }
    case XSIGFPE: {
        xdbg_info(
            "Floating Point Exception at #0x%.8X\n", state->cpu->regs.pc);
        break;
    }
    case XSIGILL: {
        xdbg_info(
            "Illegal Instruction at #0x%.8X\n", state->cpu->regs.pc);
        break;
    }
    default: {
        break;
    }
    }
}

void load_binary(iface_state* state, const char* filename)
{
    state->name = filename;
    state->cpu = init_xvm_cpu();
    state->bin = init_xvm_bin();
    set_RF(state->cpu, 0);
    xvm_bin_load_file(state->bin, (char*)filename);
    add_section(state->bin->x_section, "stack", XVM_STACK_SIZE, XVM_DFLT_SP & 0xfffff000, PERM_READ | PERM_WRITE);
    state->cpu->regs.pc = state->bin->x_header->x_entry; // set pc to entry point
    state->cpu->regs.sp = XVM_DFLT_SP;
}

void unload_binary(iface_state* state)
{
    if (state->bps) {
        delete_breakpoints(state->bps, state->bin->x_section);
        state->bps = NULL;
    }
    if (state->cpu) {
        fini_xvm_cpu(state->cpu);
        state->cpu = NULL;
    }
    if (state->bin) {
        fini_xvm_bin(state->bin);
        state->bin = NULL;
    }
}

void dbg_cpu(iface_state* state)
{
    u32 instr_size = 0;
    while (get_RF(state->cpu)) {
        instr_size = do_execute(state->cpu, state->bin);
        // fix all the breakpoints that are unpatched.
        patch_breakpoints(state->bps, state->bin->x_section); // this line is here to implement continue command.
        handle_signals(state);
    }
}
