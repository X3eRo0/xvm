//
// Created by X3eRo0 on 2/21/2021.
//

#ifndef XVM_SYMBOLS_H
#define XVM_SYMBOLS_H

#include "const.h"
#include "sections.h"

/*
*
* structure of xvm bytecode files
*
*  +-----------------+
*  |    XVM_MAGIC    |
*  +-----------------+
*  |   ENTRY_POINT   |
*  +-----------------+
*  |    N_SECTIONS   |
*  +-----------------+
*  |    N_SECTION    |<-----+
*  |        *        |      |--- .text, .data, etc.
*  |  SECTION INFO   |<-----+
*  +-----------------+
*  |      .text      |
*  +-----------------+
*  |      .data      |
*  +-----------------+
*
*/

typedef struct sym_entry_t {

    char* name; // pointer to symbol name string
    u32   addr; // address of symbol in binary
    struct sym_entry_t* next; // next symbol

} sym_entry;

typedef struct symtab_t {

    sym_entry*	symbols; // head of symbol table
    u32         n_symbols;

} symtab;


/* ****************************** SYM_ENTRY ***************************** */
sym_entry* init_sym_entry();
u32 set_sym_entry(sym_entry* x_sym_entry, char* name, u32 addr);
u32 show_sym_entry_info(sym_entry* x_sym_entry);
u32 fini_sym_entry(sym_entry* x_sym_entry);

/* ********************************************************************** */

/* ******************************* SYMTAB ******************************* */

symtab* init_symtab();
u32 add_symbol(symtab* x_symtab, char* symbol_name, u32 symbol_addr);
u32 del_symbol(symtab* x_symtab, char* symbol_name, u32 symbol_addr);
u32 write_symtab_to_file(symtab* x_symtab, section* sections, FILE* file);
u32 resolve_symbol_addr(symtab* x_symtab, char* symbol_name);
char* resolve_symbol_name(symtab* x_symtab, u32 symbol_addr);
u32 show_symtab_info(symtab* x_symtab);
u32 fini_symtab(symtab * x_symtab);

/* ********************************************************************** */

#endif //XVM_SYMBOLS_H
