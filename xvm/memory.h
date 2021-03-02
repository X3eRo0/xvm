//
// Created by X3eRo0 on 2/21/2021.
//

#ifndef XVM_MEMORY_H
#define XVM_MEMORY_H

#include "../common/const.h"

typedef struct m_map_t {

    char* m_name; // mapping name (optional)
    char* m_file; // mapping from file
    char* m_buff; // mapping buffer
    u32   v_size; // mapping virtual size
    u32   a_size; // actual size in file
    u32   v_addr; // mapping virtual address
    u32   m_prot; // mapping protections
    u32   m_ofst; // mapping offset in file
    struct m_map_t* m_next;

} m_map;

m_map*	init_m_map();
m_map*	allocate_map(m_map* map_head, char* name, char* file, u32 v_size, u32 a_size, u32 v_addr, u32 prot, u32 ofst);
u32		vmmap(m_map* map_head);
m_map*  get_map_by_addr(m_map* map_head, u32 addr);
char*   reference(m_map* map_head, u32 addr);
u8      read_byte(m_map* map_head, u32 addr, u32 opt_perm);
u16     read_word(m_map* map_head, u32 addr, u32 opt_perm);
u32     read_dword(m_map* map_head, u32 addr, u32 opt_perm);
u8      write_byte(m_map* map_head, u32 addr, u8 byte);
u16     write_word(m_map* map_head, u32 addr, u16 word);
u32     write_dword(m_map* map_head, u32 addr, u32 dword);
u32		deallocate_map(m_map* map_head, u32 v_addr);
u32		fini_m_map(m_map* map_head);


#endif //XVM_MEMORY_H
