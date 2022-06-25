#include <commands.h>
#include <stdio.h>

void xdbg_parse_args(char* cmd, char* args[])
{
    char* temp = cmd;
    u32 tlen = strlen(cmd);
    int i = 0;
    for (; i < IFACE_MAX_CMD_ARGS; i++) {
        clear_whitespaces(temp);
        args[i] = temp;
        skip_to_whitespace(temp);
        if (temp[0] == '\0') {
            break;
        } else {
            temp[0] = '\0';
            temp++;
        }
    }
    args[i + 1] = NULL;
}

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

void xdbg_hexdump(FILE* fp, const char* buffer, u32 size, u32 type, u32 address)
{
    char dump_buffer[256] = { 0 };
    char byte_buffer[256] = { 0 };
    char* temp = NULL;
    u32 ret = 0;
    // traverse in 16 byte blocks
    for (u32 block = 0; block < (size / 16); block++) {
        ret = 0;
        temp = byte_buffer;
        for (u32 i = 0; i < 16 && temp < byte_buffer + sizeof(byte_buffer); i++) {
            if (buffer[(block * 16) + i] >= 0x20 && buffer[(block * 16) + i] <= 0x7E) {
                ret = snprintf(temp, 20, KYEL "%c" KNRM, buffer[(block * 16) + i]);
                temp += ret;
            } else {
                ret = snprintf(temp, 20, KCYN "." KNRM);
                temp += ret;
            }
        }
        switch (type) {
        case TYPE_BYTES: { // byte hexdump
            ret = 0;
            temp = dump_buffer;
            for (u32 i = 0; i < 16 && temp < dump_buffer + sizeof(dump_buffer); i++) {
                ret = snprintf(temp, 10, "0x%.2X ", *((u8*)&buffer[(block * 16) + i]));
                temp += ret;
            }
            break;
        }
        case TYPE_WORDS: { // word hexdump
            ret = 0;
            temp = dump_buffer;
            for (u32 i = 0; i < 16 && temp < dump_buffer + sizeof(dump_buffer); i += 2) {
                ret = snprintf(temp, 10, "0x%.4X  ", *((u16*)&buffer[(block * 16) + i]));
                temp += ret;
            }
            break;
        }
        case TYPE_DWORD: { // dword hexdump
            ret = 0;
            temp = dump_buffer;
            for (u32 i = 0; i < 16 && temp < dump_buffer + sizeof(dump_buffer); i += 4) {
                ret = snprintf(temp, 20, "0x%.8X    ", *((u32*)&buffer[(block * 16) + i]));
                temp += ret;
            }
            break;
        }
        }
        switch (type) {
        case TYPE_BYTES:
            fprintf(fp, KGRN "0x%.8X" KNRM "\t%-80s      |%s|\n", address + (block * 16), dump_buffer, byte_buffer);
            break;
        case TYPE_WORDS:
            fprintf(fp, KGRN "0x%.8X" KNRM "\t%-80s      |%s|\n", address + (block * 16), dump_buffer, byte_buffer);
            break;
        case TYPE_DWORD:
            fprintf(fp, KGRN "0x%.8X" KNRM "\t%-80s      |%s|\n", address + (block * 16), dump_buffer, byte_buffer);
            break;
        }
    }

    // traverse the remainder bytes (if any)
    if (size % 16 != 0) {
        ret = 0;
        temp = NULL;
        memset(byte_buffer, 0, sizeof(byte_buffer));
        memset(dump_buffer, 0, sizeof(dump_buffer));
        ret = 0;
        temp = byte_buffer;
        for (u32 i = 0; i < 16 && temp < byte_buffer + sizeof(byte_buffer); i++) {
            if (i < size % 16) {
                if (buffer[size - (size % 16) + i] >= 0x20 && buffer[size - (size % 16) + i] <= 0x7E) {
                    ret = snprintf(temp, 20, KYEL "%c" KNRM, buffer[size - (size % 16) + i]);
                    temp += ret;
                } else {
                    ret = snprintf(temp, 20, KCYN "." KNRM);
                    temp += ret;
                }
            } else {
                ret = snprintf(temp, 20, " ");
                temp += ret;
            }
        }
        switch (type) {
        case TYPE_BYTES: {
            ret = 0;
            temp = dump_buffer;
            for (u32 i = 0; i < (size % 16) && temp < dump_buffer + sizeof(dump_buffer); i++) {
                ret = snprintf(temp, 10, "0x%.2X ", *((u8*)&buffer[size - (size % 16) + i]));
                temp += ret;
            }
            break;
        }
        case TYPE_WORDS: {
            ret = 0;
            temp = dump_buffer;
            for (u32 i = 0; i < (size % 16) && temp < dump_buffer + sizeof(dump_buffer); i += 2) {
                ret = snprintf(temp, 10, "0x%.4X  ", *((u16*)&buffer[size - (size % 16) + i]));
                temp += ret;
            }
            if ((size % 16) % 2 != 0) {
                ret = snprintf(temp, 10, "0x%.4X  ", (u16) * (u8*)&buffer[size - 1]);
                temp += ret;
            }
            break;
        }
        case TYPE_DWORD: {
            ret = 0;
            temp = dump_buffer;
            if (size % 16 > 4) {
                for (u32 i = 0; i < (size % 16) - ((size % 16) % 4)
                     && temp < dump_buffer + sizeof(dump_buffer);
                     i += 4) {
                    ret = snprintf(temp, 20, "0x%.8X    ", *((u32*)&buffer[size - (size % 16) + i]));
                    temp += ret;
                }
                if (((size % 16) % 4) != 0) {
                    ret = 0;
                    for (u32 i = (size) - ((size % 16) % 4), j = 0; i < size; i++, j++) {
                        ret |= buffer[i] << (j * 8);
                    }
                    ret = snprintf(temp, 20, "0x%.8X    ", ret);
                    temp += ret;
                }
            } else {
                ret = 0;
                for (u32 i = 0; i < (size % 16); i++) {
                    ret |= buffer[size - (size % 16) + i] << (i * 8);
                }
                ret = snprintf(temp, 20, "0x%.8X    ", ret);
                temp += ret;
            }
            break;
        }
        }
        switch (type) {
        case TYPE_BYTES:
            fprintf(fp, KGRN "0x%.8X" KNRM "\t%-80s      |%s|\n", address + (size - size % 16), dump_buffer, byte_buffer);
            break;
        case TYPE_WORDS:
            fprintf(fp, KGRN "0x%.8X" KNRM "\t%-80s      |%s|\n", address + (size - size % 16), dump_buffer, byte_buffer);
            break;
        case TYPE_DWORD:
            fprintf(fp, KGRN "0x%.8X" KNRM "\t%-80s      |%s|\n", address + (size - size % 16), dump_buffer, byte_buffer);
            break;
        }
    }
}

void xdbg_print_register(iface_state* state, xvm_registers regid)
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
        printf("%s%s" KNRM ": %s#0x%.8X" KNRM " (" KGRN "%s" KNRM ")\n", reg_color, regstr, val_color, val, symbol);
    } else {
        printf("%s%s" KNRM ": %s#0x%.8X" KNRM "\n", reg_color, regstr, val_color, val);
    }
}
