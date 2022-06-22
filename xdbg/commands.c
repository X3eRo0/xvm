#include "sections.h"
#include "symbols.h"
#include <commands.h>
#include <stdio.h>
#include <stdlib.h>
#include <xasm.h>

u32 cmd_help(iface_state* state, const char* command)
{
    xdbg_info("Help Menu:\n");
    printf("\n      Command        Description\n\n");
    for (u32 i = 0; cmds[i].cmd != NULL; i++) {
        printf("  [%.2d] %s%-10.10s%s\n", i + 1, cmds[i].cmd, " ", cmds[i].desc);
    }
    putchar(10);
    return E_OK;
}

u32 cmd_regs(iface_state* state, const char* command)
{
    xvm_cpu* cpu = state->cpu;
    for (u32 _reg = reg_r0; _reg < XVM_NREGS; _reg++) {
        print_register(state, _reg);
    }
    return E_OK;
}

u32 cmd_disasm(iface_state* state, const char* command)
{
    // try to parse as a symbol. if no symbol exist then
    // try to parse as number.

    char* argument = (char*)command + 6;
    clear_whitespaces(argument);
    char* ninstr_s = argument;
    skip_to_whitespace(ninstr_s);
    u32 ninstr = 5;
    if (*ninstr_s != '\0') {
        *ninstr_s++ = '\0';
        clear_whitespaces(ninstr_s);
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
    xasm_disassemble_bytes_colored(stdout, state->bin, bytecode, len, symaddr, ninstr, 1);
    return E_OK;
}

u32 cmd_exit(iface_state* state, const char* command)
{
    xdbg_info("Exitting xdbg.\n");
    state->rflag ^= 1;
    return E_OK;
}

u32 cmd_invalid(iface_state* state, const char* command)
{
    (void)state;
    (void)command;
    xdbg_error("Invalid command.\n");
    return E_OK;
}
