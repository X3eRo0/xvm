#include "const.h"
#include "sections.h"
#include <breakpoints.h>
#include <commands.h>
#include <xasm.h>

u32 cmd_clear(iface_state* state, const char* args[])
{
    printf("\033[H\x1b[J");
    return E_OK;
}

u32 cmd_help(iface_state* state, const char* args[])
{
    xdbg_info("Help Menu:\n");
    printf("\n       commands        Description\n\n");
    for (u32 i = 0; cmds[i].cmd != NULL; i++) {
        printf("  [%.2d] %-16s%s\n", i + 1, cmds[i].cmd, cmds[i].desc);
    }
    putchar(10);
    return E_OK;
}

u32 cmd_info(iface_state* state, const char* args[])
{
    if (state->bin) {
        xvm_bin_show_exe_info(state->bin);
        return E_OK;
    } else {
        xdbg_error("Please load a binary first\n");
        return E_ERR;
    }
}

u32 cmd_load(iface_state* state, const char* args[])
{
    if (args[1] == NULL) {
        xdbg_error("Missing arguments\n  Arguments:\n    <Mandatary : full path for the file to load>\n");
        return E_ERR;
    }
    unload_binary(state);
    load_binary(state, args[1]);
    return E_OK;
}

u32 cmd_regs(iface_state* state, const char* args[])
{
    xvm_cpu* cpu = state->cpu;
    for (u32 _reg = reg_r0; _reg < XVM_NREGS; _reg++) {
        xdbg_print_register(state, _reg);
    }
    return E_OK;
}

u32 cmd_tele(iface_state* state, const char* args[])
{
    if (args[1] && !strncmp(args[1], "help", 4)) {
        xdbg_info("Missing arguments\n  Arguments:\n    <Mandatary : address>\n    <Optional  : # of pointers to display\n");
        return E_OK;
    }

    u32 nptrs = 5; // default number of pointers to show
    u32 address = state->cpu->regs.sp;

    if (args[1]) {
        if (args[1][0] == '$') { // parse as register
            for (u32 i = reg_r0; i <= reg_sp; i++) {
                if (!strncmp(args[1], regid_2_str[i], strlen(regid_2_str[i]))) {
                    // print register
                    address = ((u32*)&state->cpu->regs)[i];
                }
            }
            if (address == 0) {
                xdbg_error("Invalid register \"%s\".\n", args[1]);
                return E_ERR;
            }
        } else {
            if (resolve_number((char*)args[1], &address) == E_ERR) {
                xdbg_error("Invalid number \"%s\".\n", args[1]);
                return E_ERR;
            }
        }
    }

    if (args[2] && resolve_number((char*)args[2], &nptrs) == E_ERR) {
        xdbg_error("Invalid number \"%s\".\n", args[2]);
        return E_ERR;
    }

    for (int i = 1; i < nptrs; i++) {

        section_entry* sec = find_section_entry_by_addr(state->bin->x_section, address);
        if (sec == NULL) {
            xdbg_error("Unmapped address: #0x%.8x.\n", address);
            return E_ERR;
        }

        char* symbol = resolve_symbol_name(state->bin->x_symtab, address);
        u32 value = read_dword(state->bin->x_section, address, PERM_READ);
        if (symbol != NULL) {
            printf("  " KYEL "#0x%.8X " KNRM ":    " KBLU "0x%.8X" KRED " ->" KNRM " (" KGRN "%s" KNRM ")\n", address, value, symbol);
        } else {
            printf("  " KYEL "#0x%.8X " KNRM ":    " KBLU "0x%.8X" KNRM "\n", address, value);
        }

        address += i * 4;
    }

    return E_OK;
}

u32 cmd_context(iface_state* state, const char* args[])
{
    xdbg_info("Registers\n");
    cmd_regs(state, args);
    xdbg_info("Code\n");
    section_entry* sec = find_section_entry_by_addr(state->bin->x_section, state->cpu->regs.pc);
    if (sec == NULL) {
        xdbg_error("Cannot disassemble $pc\n");
    } else {
        unpatch_breakpoints(state->bps, state->bin->x_section);
        xasm_disassemble_bytes_colored(stdout, state->bin, sec->m_buff + state->cpu->regs.pc - sec->v_addr, sec->v_size - (state->cpu->regs.pc - sec->v_addr), state->cpu->regs.pc, 5, 1);
        patch_breakpoints(state->bps, state->bin->x_section);
    }
    xdbg_info("Stack\n");
    cmd_tele(state, args);
    return E_OK;
}

u32 cmd_vmmap(iface_state* state, const char* args[])
{
    show_section_info(state->bin->x_section);
    return E_OK;
}

u32 cmd_set(iface_state* state, const char* args[])
{
    if (args[1] == NULL || args[2] == NULL || args[3] == NULL) {
        xdbg_error("Missing arguments\n  Arguments:\n    <Mandatary  : address/register>\n    <Mandatary  : width (Byte,Word/Half,Dword)>\n    <Mandatary  : Value (u32)>\n");
        return E_ERR;
    }

    u32 value = 0;
    if (resolve_number((char*)args[3], &value) == E_ERR) {
        xdbg_error("Invalid number \"%s\".\n", args[3]);
        return E_ERR;
    }

    u32 type = 0;
    char* temp_buffer = strdup(args[2]);

    for (int i = 0; i < strlen(temp_buffer); i++) {
        temp_buffer[i] |= ' '; // make it lowercase
    }
    if (!strncmp(temp_buffer, "b", 1)) {
        type = TYPE_BYTES;
    } else if (!strncmp(temp_buffer, "h", 1) || !strncmp(temp_buffer, "w", 1)) {
        type = TYPE_WORDS;
    } else if (!(strncmp(temp_buffer, "d", 1))) {
        type = TYPE_DWORD;
    } else {
        xdbg_error("Invalid format \"%s\".\n", args[2]);
    }
    free(temp_buffer);
    temp_buffer = NULL;
    void* buffer = NULL;
    u32 address = 0;
    if (args[1][0] == '$') { // parse as register
        for (u32 i = reg_r0; i <= reg_sp; i++) {
            if (!strncmp(args[1], regid_2_str[i], strlen(regid_2_str[i]))) {
                // print register
                buffer = &((u32*)&state->cpu->regs)[i];
                break;
            }
        }
        if (buffer == NULL) {
            xdbg_error("Invalid register \"%s\".\n", args[1]);
            return E_ERR;
        }
    } else {
        if (resolve_number((char*)args[1], &address) == E_ERR) {
            xdbg_error("Invalid number \"%s\".\n", args[1]);
            return E_ERR;
        }
        section_entry* temp = find_section_entry_by_addr(state->bin->x_section, address);
        if (temp == NULL) {
            xdbg_error("Invalid address \"#0x%.8x\".\n", address);
            return E_ERR;
        }
        buffer = temp->m_buff + (address - temp->v_addr);
    }

    switch (type) {
    case TYPE_BYTES: {
        *(u8*)buffer = value & 0xff;
        break;
    }
    case TYPE_WORDS: {
        *(u16*)buffer = value & 0xffff;
        break;
    }
    case TYPE_DWORD: {
        *(u32*)buffer = value;
        break;
    }
    default: {
        xdbg_error("Invalid type.\n");
        return E_ERR;
    }
    }

    return E_OK;
}

u32 cmd_xamine(iface_state* state, const char* args[])
{
    if (args[1] == NULL) {
        xdbg_error("Missing arguments\n  Arguments:\n    <Mandatary  : address>\n    <Optional  : width (Byte,Word/Half,Dword,String,Instructions)>\n    <Optional  : # of Bytes>\n");
        return E_ERR;
    }
    u32 address = 0;
    u32 type = TYPE_BYTES; // default type is BYTES
    u32 len = 0x100; // default dump length is 0x100

    if (args[2] == NULL) {
        // if only arg1 and arg1 is reg then print register
        if (args[1][0] == '$') { // parse as register
            for (u32 i = reg_r0; i <= reg_sp; i++) {
                if (!strncmp(args[1], regid_2_str[i], strlen(regid_2_str[i]))) {
                    // print register
                    xdbg_print_register(state, i);
                    return E_OK;
                }
            }
            xdbg_error("Invalid register \"%s\".\n", args[1]);
            return E_ERR;
        }
    } else {
        if (args[1][0] == '$') { // parse as register
            for (u32 i = reg_r0; i <= reg_sp; i++) {
                if (!strncmp(args[1], regid_2_str[i], strlen(regid_2_str[i]))) {
                    // print register
                    address = ((u32*)&state->cpu->regs)[i];
                }
            }
            if (address == 0) {
                xdbg_error("Invalid register \"%s\".\n", args[1]);
                return E_ERR;
            }
        } else {
            if (resolve_number((char*)args[1], &address) == E_ERR) {
                xdbg_error("Invalid number \"%s\".\n", args[1]);
                return E_ERR;
            }
        }
    }

    section_entry* sec = find_section_entry_by_addr(state->bin->x_section, address);
    if (sec == NULL) {
        xdbg_error("Unmapped address: 0x%.8x.\n", address);
        return E_ERR;
    }

    if (args[2] != NULL) {
        char* temp_buffer = strdup(args[2]);

        for (int i = 0; i < strlen(temp_buffer); i++) {
            temp_buffer[i] |= ' '; // make it lowercase
        }
        if (!strncmp(temp_buffer, "b", 1)) {
            type = TYPE_BYTES;
        } else if (!strncmp(temp_buffer, "h", 1) || !strncmp(temp_buffer, "w", 1)) {
            type = TYPE_WORDS;
        } else if (!(strncmp(temp_buffer, "d", 1))) {
            type = TYPE_DWORD;
        } else if (!(strncmp(temp_buffer, "s", 1))) {
            type = TYPE_STRING;
        } else if (!strncmp(temp_buffer, "i", 1)) {
            type = TYPE_DISASM;
        }
        free(temp_buffer);
        temp_buffer = NULL;
    }

    if (args[3] != NULL) {
        if (resolve_number((char*)args[3], &len) == E_ERR) {
            xdbg_error("Invalid number \"%s\".\n", args[3]);
            return E_ERR;
        }
    }

    const char* buffer = (const char*)(sec->m_buff + address - sec->v_addr);
    if (type != TYPE_DISASM) {
        if (type == TYPE_STRING) {
            type = TYPE_BYTES;
            len = strlen(buffer);
        }
        xdbg_hexdump(stdout, buffer, len, type, address);
    } else {
        unpatch_breakpoints(state->bps, state->bin->x_section);
        xasm_disassemble_bytes_colored(stdout, state->bin, buffer, len, address, 0, 1);
        patch_breakpoints(state->bps, state->bin->x_section);
    }
    return E_OK;
}

u32 cmd_disasm(iface_state* state, const char* args[])
{
    // try to parse as a symbol. if no symbol exist then
    // try to parse as number.

    if (args[1] == NULL) {
        xdbg_error("Missing arguments\n  Arguments:\n    <Mandatary  : address>\n    <Optional  : number of instructions>\n");
        return E_ERR;
    }

    char* argument = (char*)args[1];
    char* ninstr_s = (char*)args[2];
    u32 ninstr = 5;
    if (ninstr_s != NULL) {
        ninstr = atoi(ninstr_s);
    }
    u32 symaddr = resolve_symbol_addr(state->bin->x_symtab, argument);
    if (symaddr == E_ERR) {
        if (resolve_number(argument, &symaddr) == E_ERR) {
            xdbg_error("No function \"%s\" in the current context.\n");
            return E_ERR;
        }
    }
    section_entry* sec = find_section_entry_by_addr(state->bin->x_section, symaddr);
    if (sec == NULL) {
        xdbg_error("Unmapped address: 0x%.8x\n", symaddr);
        return E_ERR;
    }
    const char* bytecode = (const char*)(sec->m_buff + symaddr - sec->v_addr);
    u32 len = sec->v_addr + sec->v_size - symaddr;
    unpatch_breakpoints(state->bps, state->bin->x_section);
    xasm_disassemble_bytes_colored(stdout, state->bin, bytecode, len, symaddr, ninstr, 1);
    patch_breakpoints(state->bps, state->bin->x_section);
    return E_OK;
}

u32 cmd_exit(iface_state* state, const char* args[])
{
    xdbg_info("Exiting xdbg.\n");
    state->rflag ^= 1;
    return E_OK;
}

u32 cmd_invalid(iface_state* state, const char* args[])
{
    (void)state;
    (void)args;
    xdbg_error("Invalid command.\n");
    return E_OK;
}

u32 cmd_run(iface_state* state, const char* args[])
{
    set_RF(state->cpu, 1);
    return E_OK;
}

u32 cmd_continue(iface_state* state, const char* args[])
{
    unpatch_breakpoint_by_addr(state->bps, state->bin->x_section, state->cpu->regs.pc);
    set_RF(state->cpu, 1);
    return E_OK;
}

u32 cmd_stop(iface_state* state, const char* args[])
{
    set_RF(state->cpu, 0);
    return E_OK;
}

u32 cmd_break(iface_state* state, const char* args[])
{
    if (args[1] == NULL) {
        xdbg_error("Missing arguments\n  Arguments:\n    <Mandatary  : <address>\n");
        return E_ERR;
    }

    u32 address = resolve_symbol_addr(state->bin->x_symtab, (char*)args[1]);
    if (address == E_ERR) {
        if (resolve_number((char*)args[1], &address) == E_ERR) {
            xdbg_error("No function \"%s\" in the current context.\n");
            return E_ERR;
        }
    }

    if (state->bps == NULL) {
        state->bps = init_breakpoint();
    }

    if (add_breakpoint(state->bps, state->bin->x_section, address) == E_ERR) {
        xdbg_error("Unable to set breakpoint at \"%s\".\n", args[1]);
        return E_ERR;
    }
    return E_OK;
}

u32 cmd_showbp(iface_state* state, const char* args[])
{
    show_breakpoints(state->bps, state->bin->x_symtab);
    return E_OK;
}

u32 cmd_disable(iface_state* state, const char* args[])
{
    if (args[1] == NULL) {
        xdbg_error("Missing arguments\n  Arguments:\n    <Mandatary  : <breakpoint id/all>\n");
        return E_ERR;
    }

    if (!strncmp(args[1], "all", 3)) {
        return disable_breakpoints(state->bps, state->bin->x_section);
    }

    u32 bp_id = 0;
    if (resolve_number((char*)args[1], &bp_id) == E_ERR) {
        xdbg_error("Invalid number \"%s\".\n", args[1]);
        return E_ERR;
    }

    return disable_breakpoint_by_id(state->bps, state->bin->x_section, bp_id);
}

u32 cmd_enable(iface_state* state, const char* args[])
{
    if (args[1] == NULL) {
        xdbg_error("Missing arguments\n  Arguments:\n    <Mandatary  : <breakpoint id/all>\n");
        return E_ERR;
    }

    if (!strncmp(args[1], "all", 3)) {
        return enable_breakpoints(state->bps, state->bin->x_section);
    }

    u32 bp_id = 0;
    if (resolve_number((char*)args[1], &bp_id) == E_ERR) {
        xdbg_error("Invalid number \"%s\".\n", args[1]);
        return E_ERR;
    }

    return enable_breakpoint_by_id(state->bps, state->bin->x_section, bp_id);
}

u32 cmd_delete(iface_state* state, const char* args[])
{
    if (args[1] == NULL) {
        xdbg_error("Missing arguments\n  Arguments:\n    <Mandatary  : <breakpoint id/all>\n");
        return E_ERR;
    }

    if (!strncmp(args[1], "all", 3)) {
        return delete_breakpoints(state->bps, state->bin->x_section);
    }

    u32 bp_id = 0;
    if (resolve_number((char*)args[1], &bp_id) == E_ERR) {
        xdbg_error("Invalid number \"%s\".\n", args[1]);
        return E_ERR;
    }

    u32 ret = delete_breakpoint_by_id(state->bps, state->bin->x_section, bp_id);

    if (bp_id == state->bps->id) {
        state->bps = state->bps->next;
    }

    return ret;
}
