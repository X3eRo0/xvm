//
// Created by X3eRo0 on 2/21/2021.
//

#include <symbols.h>

/* ****************************** SYM_ENTRY ***************************** */

sym_entry* init_sym_entry()
{
    // allocate sym entry structure on heap and initialize
    // name, addr and next member

    sym_entry* x_sym_entry = (sym_entry*)malloc(sizeof(sym_entry));

    if (x_sym_entry == NULL) {
        return NULL;
    }

    x_sym_entry->addr = 0;
    x_sym_entry->name = NULL;
    x_sym_entry->next = NULL;

    return x_sym_entry;
}

u32 set_sym_entry(sym_entry* x_sym_entry, char* name, u32 addr)
{
    x_sym_entry->name = strdup(name);
    x_sym_entry->addr = addr;
    x_sym_entry->next = NULL;

    return E_OK;
}

u32 show_sym_entry_info(sym_entry* x_sym_entry)
{
    // print symbol entry

    if (x_sym_entry == NULL || x_sym_entry->name == NULL) {
        return E_ERR;
    }

    printf(KBLU "#0x%.8x    " KGRN "%s\n" KNRM, x_sym_entry->addr, x_sym_entry->name);
    return E_OK;
}

u32 fini_sym_entry(sym_entry* x_sym_entry)
{

    if (x_sym_entry != NULL) {

        free(x_sym_entry->name);
        x_sym_entry->name = NULL;
        x_sym_entry->addr = 0x00;
        x_sym_entry->next = NULL;
        free(x_sym_entry);
        x_sym_entry = NULL;

        return E_OK;
    }

    return E_ERR;
}

/* ********************************************************************** */
/* ******************************* SYMTAB ******************************* */

symtab* init_symtab()
{
    // symtab constructor
    // allocate symbol table structure on heap and initialize
    // symbols member to NULL

    symtab* x_symtab = (symtab*)malloc(sizeof(symtab));

    if (x_symtab == NULL) {
        return NULL;
    }

    x_symtab->symbols = NULL;
    x_symtab->n_symbols = 0;

    return x_symtab;
}

u32 add_symbol(symtab* x_symtab, char* symbol_name, u32 symbol_addr)
{
    // insert new symbol entry in symbol table

    x_symtab->n_symbols++;

    if (x_symtab->symbols == NULL) {
        x_symtab->symbols = init_sym_entry();
        set_sym_entry(x_symtab->symbols, symbol_name, symbol_addr);
        return E_OK;
    }

    sym_entry* temp = x_symtab->symbols;

    // reach the end of list
    while (temp->next != NULL) {
        temp = temp->next;
    }

    temp->next = init_sym_entry();
    set_sym_entry(temp->next, symbol_name, symbol_addr);

    return E_OK;
}

u32 write_symtab_to_file(symtab* x_symtab, section* sections, FILE* file)
{

    section_entry* data_s = find_section_entry_by_name(sections, ".data");
    if (data_s == NULL) {
        return E_ERR;
    }
    sym_entry* temp = x_symtab->symbols;

    while (temp != NULL) {
        fwrite(&data_s->m_ofst, sizeof(u32), 1, file); // wrote the offset into section
        fwrite(&temp->addr, sizeof(u32), 1, file); // wrote the symbol address
        memcpy_to_buffer(sections, data_s, temp->name, strlen(temp->name) + 1);
        temp = temp->next;
    }

    return E_OK;
}

u32 del_symbol(symtab* x_symtab, char* symbol_name, u32 symbol_addr)
{
    // delete a symbol entry from symbol table
    // using symbol name and address

    x_symtab->n_symbols--;

    // temp entry to traverse symbol table
    sym_entry* temp = x_symtab->symbols;
    sym_entry* prev = NULL;

    u32 s_len = strlen(symbol_name);

    // find the sym_entry whose name field
    // matches the symbol_name argument

    while (temp != NULL) {
        if (!strncmp(temp->name, symbol_name, s_len)
            && temp->addr == symbol_addr) {
            break;
        }
        prev = temp;
        temp = temp->next;
    }

    // if temp == NULL symbol not found

    if (temp == NULL) {
        return E_ERR;
    }

    if (prev == NULL) {
        return E_ERR;
    }

    prev->next = NULL;
    free(temp);
    temp = NULL;

    return E_OK;
}

u32 resolve_symbol_addr(symtab* x_symtab, char* symbol_name)
{
    // get address using symbol name

    // temp entry to traverse symbol table
    sym_entry* temp = x_symtab->symbols;
    u32 s_len = strlen(symbol_name);

    // find the sym_entry whose name field
    // matches the symbol_name argument

    while (temp != NULL) {
        if (!strncmp(temp->name, symbol_name, s_len)) {
            break;
        }
        temp = temp->next;
    }

    // if temp == NULL symbol not found

    if (temp == NULL) {
        return E_ERR;
    }

    return temp->addr; // return the address
}

char* resolve_symbol_name(symtab* x_symtab, u32 symbol_addr)
{
    // delete a symbol entry from symbol table
    // using symbol name and address

    // temp entry to traverse symbol table
    sym_entry* temp = x_symtab->symbols;

    // find the sym_entry whose name field
    // matches the symbol_name argument

    while (temp != NULL) {
        if (temp->addr == symbol_addr) {
            break;
        }
        temp = temp->next;
    }

    // if temp == NULL symbol not found

    if (temp == NULL) {
        return NULL;
    }

    return temp->name; // return symbol name
}

u32 show_symtab_info(symtab* x_symtab)
{
    // print the symbol table
    // FIXME : COMMENT FOR RELEASE

    printf("\n[" KGRN "+" KNRM "] Dumping SymTab\n");
    printf("  Address      Symbol\n");

    // temp entry to traverse symbol table
    sym_entry* temp = x_symtab->symbols;

    while (temp != NULL) {
        show_sym_entry_info(temp);
        temp = temp->next;
    }

    return E_OK;
}

u32 fini_symtab(symtab* x_symtab)
{

    // symtab destructor

    sym_entry* temp = x_symtab->symbols;
    sym_entry* prev = NULL;

    while (temp != NULL) {
        prev = temp;
        temp = temp->next;
        fini_sym_entry(prev);
    }

    x_symtab->symbols = NULL;
    free(x_symtab);
    x_symtab = NULL;
    return E_OK;
}

/* ********************************************************************** */
