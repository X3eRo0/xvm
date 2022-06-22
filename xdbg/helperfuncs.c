#include <commands.h>

void xdbg_info(char* fmt, ...)
{
    va_list valist;
    printf("[" KGRN "+" KNRM "] ");
    va_start(valist, fmt);
    vprintf(fmt, valist);
    va_end(valist);
}

void xdbg_error(char* fmt, ...)
{
    va_list valist;
    printf("[" KRED "!" KNRM "] ");
    va_start(valist, fmt);
    vprintf(fmt, valist);
    va_end(valist);
}

void print_register(iface_state* state, xvm_registers regid)
{
    const char* regstr = regid_2_str[regid];
    u32 val = ((u32*)&state->cpu->regs)[regid];
    const char* val_color = KNRM;
    const char* reg_color = KNRM;

    section_entry* temp = find_section_entry_by_addr(state->bin->x_section, val);
    if (temp != NULL) {

        if (temp->m_flag & PERM_EXEC) {
            // executable memory marked RED
            val_color = KRED;
        } else if (temp->m_flag & PERM_WRITE) {
            // writable memory marked cyan
            val_color = KCYN;
        }
    }

    // special cases
    if (val == 0) {
        val_color = KBLU;
    }

    if (val >= XVM_DFLT_SP && val < XVM_DFLT_SP + XVM_STACK_SIZE) {
        val_color = KYEL;
    }

    if (regid == reg_pc) {
        reg_color = DRED;
    }

    if (regid == reg_bp || regid == reg_sp) {
        reg_color = KMAG;
    }
    // check if val is in the symbol table
    char* symbol = resolve_symbol_name(state->bin->x_symtab, val);
    if (symbol != NULL) {
        printf("%s%s" KNRM ": %s0x%.8X" KNRM " (" KGRN "%s" KNRM ")\n", reg_color, regstr, val_color, val, symbol);
    } else {
        printf("%s%s" KNRM ": %s0x%.8X" KNRM "\n", reg_color, regstr, val_color, val);
    }
}
