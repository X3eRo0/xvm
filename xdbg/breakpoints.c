#include <breakpoints.h>
#include <commands.h>
#include <symbols.h>

static u32 breakpoint_id = 0;

breaklist* init_breakpoint()
{
    breaklist* ptr = (breaklist*)calloc(1, sizeof(breaklist));
    return ptr;
}

u32 enable_breakpoint_t(breaklist* head, section* sec)
{
    if (head == NULL) {
        return E_ERR;
    }

    if (head->enabled) {
        return E_OK;
    }

    section_entry* sec_entry = find_section_entry_by_addr(sec, head->address);
    if (sec_entry == NULL) {
        return E_ERR;
    }

    // patching memory
    patch_breakpoint_by_addr(head, sec, head->address);
    return E_OK;
}

u32 enable_breakpoint_by_addr(breaklist* head, section* sec, u32 address)
{
    if (head == NULL) {
        return E_ERR;
    }

    breaklist* temp = head;
    while (temp->address != address && temp != NULL) {
        temp = temp->next;
    }

    if (temp == NULL) {
        return E_ERR;
    }

    return enable_breakpoint_t(temp, sec);
}

u32 enable_breakpoint_by_id(breaklist* head, section* sec, u32 id)
{
    if (head == NULL) {
        return E_ERR;
    }

    breaklist* temp = head;
    while (temp->address != id && temp != NULL) {
        temp = temp->next;
    }

    if (temp == NULL) {
        return E_ERR;
    }

    return enable_breakpoint_t(temp, sec);
}

u32 enable_breakpoints(breaklist* head, section* sec)
{
    if (head == NULL) {
        return E_ERR;
    }

    breaklist* temp = head;
    breaklist* next = NULL;
    while (temp->next != NULL) {
        next = temp->next;
        if (enable_breakpoint_t(temp, sec) == E_ERR) {
            return E_ERR;
        }
        temp = next;
    }

    return E_OK;
}

u32 disable_breakpoint_t(breaklist* head, section* sec)
{
    if (head == NULL) {
        return E_ERR;
    }

    if (!head->enabled) {
        return E_OK;
    }

    section_entry* sec_entry = find_section_entry_by_addr(sec, head->address);
    if (sec_entry == NULL) {
        return E_ERR;
    }

    // patching memory
    unpatch_breakpoint_t(head, sec);
    head->enabled = 0;
    return E_OK;
}

u32 disable_breakpoint_by_addr(breaklist* head, section* sec, u32 address)
{
    if (head == NULL) {
        return E_ERR;
    }

    breaklist* temp = head;
    while (temp->address != address && temp != NULL) {
        temp = temp->next;
    }

    if (temp == NULL) {
        return E_ERR;
    }

    return disable_breakpoint_t(temp, sec);
}

u32 disable_breakpoint_by_id(breaklist* head, section* sec, u32 id)
{
    if (head == NULL) {
        return E_ERR;
    }

    breaklist* temp = head;
    while (temp != NULL && temp->id != id) {
        temp = temp->next;
    }

    if (temp == NULL) {
        return E_ERR;
    }

    return disable_breakpoint_t(temp, sec);
}

u32 disable_breakpoints(breaklist* head, section* sec)
{
    if (head == NULL) {
        return E_ERR;
    }

    breaklist* temp = head;
    while (temp != NULL) {
        if (disable_breakpoint_t(temp, sec) == E_ERR) {
            return E_ERR;
        }
        temp = temp->next;
    }

    return E_OK;
}

u32 add_breakpoint(breaklist* head, section* sec, u32 address)
{
    if (head == NULL) {
        return E_ERR;
    }

    breaklist* temp = head;
    if (head->address != 0 && head->id != 0) {
        while (temp->next != NULL) {
            if (temp->address == address) {
                return E_ERR;
            }
            temp = temp->next;
        }
        temp->next = init_breakpoint();
        temp = temp->next;
    }

    section_entry* sec_entry = find_section_entry_by_addr(sec, address);
    if (sec_entry == NULL) {
        return E_ERR;
    }

    temp->id = ++breakpoint_id;
    temp->address = address;
    temp->enabled = 1;
    temp->next = NULL;

    // patching memory
    patch_breakpoint_t(temp, sec);

    xdbg_info("Breakpoint set #%d\n", temp->id);

    return E_OK;
}

u32 patch_breakpoint_t(breaklist* head, section* sec)
{
    if (head == NULL || sec == NULL) {
        return E_ERR;
    }

    if (!head->patched) {

        if (get_word(sec, head->address, &head->op) == E_ERR) {
            return E_ERR;
        }

        if (set_word(sec, head->address, XVM_OP_TRAP) == E_ERR) {
            return E_ERR;
        }

        head->patched = 1;
    }

    return E_OK;
}

u32 patch_breakpoint_by_addr(breaklist* head, section* sec, u32 address)
{
    if (head == NULL) {
        return E_ERR;
    }

    breaklist* temp = head;
    while (temp != NULL) {
        if (temp->address == address) {
            break;
        }
        temp = temp->next;
    }

    if (temp == NULL) {
        return E_ERR;
    }

    return patch_breakpoint_t(temp, sec);
}

u32 patch_breakpoints(breaklist* head, section* sec)
{
    if (head == NULL) {
        return E_ERR;
    }

    breaklist* temp = head;
    while (temp != NULL) {
        if (patch_breakpoint_t(temp, sec) == E_ERR) {
            return E_ERR;
        }
        temp = temp->next;
    }

    return E_OK;
}

u32 unpatch_breakpoint_t(breaklist* head, section* sec)
{
    if (head == NULL || sec == NULL) {
        return E_ERR;
    }

    if (head->patched) {

        if (set_word(sec, head->address, head->op) == E_ERR) {
            return E_ERR;
        }

        head->patched = 0;
    }

    return E_OK;
}

u32 unpatch_breakpoint_by_id(breaklist* head, section* sec, u32 id)
{
    if (head == NULL) {
        return E_ERR;
    }

    breaklist* temp = head;
    breaklist* prev = NULL;
    while (temp != NULL) {
        if (temp->id == id) {
            break;
        }
        prev = temp;
        temp = temp->next;
    }

    if (temp == NULL) {
        return E_ERR;
    }

    return unpatch_breakpoint_t(temp, sec);
}

u32 unpatch_breakpoint_by_addr(breaklist* head, section* sec, u32 address)
{
    if (head == NULL) {
        return E_ERR;
    }

    breaklist* temp = head;
    while (temp != NULL) {
        if (temp->address == address) {
            break;
        }
        temp = temp->next;
    }

    if (temp == NULL) {
        return E_ERR;
    }

    return unpatch_breakpoint_t(temp, sec);
}

u32 unpatch_breakpoints(breaklist* head, section* sec)
{
    if (head == NULL) {
        return E_ERR;
    }

    breaklist* temp = head;
    while (temp != NULL) {
        if (unpatch_breakpoint_t(temp, sec) == E_ERR) {
            return E_ERR;
        }
        temp = temp->next;
    }

    return E_OK;
}

u32 delete_breakpoint_by_addr(breaklist* head, section* sec, u32 address)
{
    if (head == NULL) {
        return E_ERR;
    }

    breaklist* temp = head;
    breaklist* prev = NULL;
    while (temp != NULL) {
        if (temp->address == address) {
            break;
        }
        prev = temp;
        temp = temp->next;
    }

    if (temp == NULL) {
        return E_ERR;
    }

    if (prev != NULL) {
        prev->next = temp->next;
    }

    return delete_breakpoint_t(temp, sec);
}

u32 delete_breakpoint_by_id(breaklist* head, section* sec, u32 id)
{
    if (head == NULL) {
        return E_ERR;
    }

    breaklist* temp = head;
    breaklist* prev = NULL;
    while (temp != NULL) {
        if (temp->id == id) {
            break;
        }
        prev = temp;
        temp = temp->next;
    }

    if (temp == NULL) {
        return E_ERR;
    }

    if (prev != NULL) {
        prev->next = temp->next;
    }

    return delete_breakpoint_t(temp, sec);
}

u32 delete_breakpoint_t(breaklist* head, section* sec)
{
    if (head == NULL) {
        return E_ERR;
    }

    section_entry* sec_entry = find_section_entry_by_addr(sec, head->address);
    if (sec_entry == NULL) {
        return E_ERR;
    }

    // patching memory
    unpatch_breakpoint_t(head, sec);
    memset(head, 0, sizeof(breaklist));
    free(head);
    head = NULL;
    return E_OK;
}

u32 delete_breakpoints(breaklist* head, section* sec)
{
    if (head == NULL) {
        return E_ERR;
    }

    breaklist* temp = head;
    breaklist* next = NULL;
    while (temp != NULL) {
        next = temp->next;
        if (delete_breakpoint_t(temp, sec) == E_ERR) {
            return E_ERR;
        }
        temp = next;
    }

    return E_OK;
}

u32 show_breakpoint_t(breaklist* head, symtab* syms)
{
    if (head == NULL) {
        return E_ERR;
    }
    char* symbol = resolve_symbol_name(syms, head->address);

    if (symbol) {
        printf("%-5d %-5s" KBLU " #0x%.8x" KNRM "     <%s>\n", head->id, (head->enabled) ? "y" : "n", head->address, symbol);
    } else {
        printf("%-5d %-5s" KBLU " #0x%.8x" KNRM "\n", head->id, (head->enabled) ? "y" : "n", head->address);
    }
    return E_OK;
}

u32 show_breakpoints(breaklist* head, symtab* syms)
{
    if (breakpoint_id == 0 || head == NULL) {
        xdbg_error("No breakpoints set.\n");
        return E_ERR;
    }

    printf("Num   Enb   Address         Symbol\n");

    breaklist* temp = head;
    while (temp != NULL) {
        if (show_breakpoint_t(temp, syms) == E_ERR) {
            return E_ERR;
        }
        temp = temp->next;
    }

    return E_OK;
}
