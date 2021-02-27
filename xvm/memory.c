//
// Created by X3eRo0 on 2/21/2021.
//

#include "memory.h"

m_map* init_m_map() {

    m_map* t_map = (m_map*)malloc(sizeof(m_map));

    if (t_map == NULL) {
        return NULL;
    }

    t_map->m_name = NULL;
    t_map->m_file = NULL;
    t_map->m_buff = NULL;
    t_map->v_size = 0x00;
    t_map->a_size = 0x00;
    t_map->v_addr = 0x00;
    t_map->m_prot = 0x00;
    t_map->m_ofst = 0x00;
    t_map->m_next = NULL;

    return t_map;
}

m_map* allocate_map(m_map* map_head, char* name, char* file, u32 v_size, u32 a_size, u32 v_addr, u32 prot, u32 ofst) {

    if (map_head == NULL) {
        map_head = init_m_map();
        map_head->m_name = name;
        map_head->m_file = file;
        map_head->m_buff = (char*)malloc(v_size);
        map_head->v_size = v_size;
        map_head->a_size = a_size;
        map_head->v_addr = v_addr;
        map_head->m_prot = prot;
        map_head->m_ofst = ofst;
        map_head->m_next = NULL;

        return map_head;
    }

    m_map* t_map = map_head;
    while (t_map->m_next != NULL) {
        if (t_map->v_addr == v_addr) {
            return NULL;
        }

        t_map = t_map->m_next;
    }

    t_map->m_next = init_m_map();
    t_map = t_map->m_next;
    t_map->m_name = name;
    t_map->m_file = file;
    t_map->m_buff = (char*)malloc(v_size);
    t_map->v_size = v_size;
    t_map->a_size = a_size;
    t_map->v_addr = v_addr;
    t_map->m_prot = prot;
    t_map->m_ofst = ofst;
    t_map->m_next = NULL;

    return t_map;
}

m_map* get_map_by_addr(m_map* map_head, u32 addr){
    // traverse the maps and check
    // which map contains this address

    m_map* t_map = map_head;
    while (t_map != NULL) {
        if (addr >= t_map->v_addr && addr <= (t_map->v_addr + t_map->v_size)) {
            return t_map;
        }
        t_map = t_map->m_next;
    }

    // FIXME: give segfault here
    return NULL;
}

char* reference(m_map* map_head, u32 addr){
    // get a reference of virtual memory

    m_map* addr_map = get_map_by_addr(map_head, addr);
    u32 offset = addr - addr_map->v_addr;

    return (char*)&addr_map->m_buff[offset];
}

u8 dereference_byte(m_map* map_head, u32 addr){
    // dereference addr as a byte ptr from virtual memory

    m_map* addr_map = get_map_by_addr(map_head, addr);
    u32 offset = addr - addr_map->v_addr;

    return (u8)addr_map->m_buff[offset];
}

u16 dereference_word(m_map* map_head, u32 addr){
    // dereference addr as a word ptr from virtual memory

    m_map* addr_map = get_map_by_addr(map_head, addr);
    u32 offset = addr - addr_map->v_addr;

    return (u16)*(u16*)&addr_map->m_buff[offset];
}

u32 dereference_dword(m_map* map_head, u32 addr){
    // dereference addr as a dword ptr from virtual memory

    m_map* addr_map = get_map_by_addr(map_head, addr);
    u32 offset = addr - addr_map->v_addr;

    return (u32)*(u32*)&addr_map->m_buff[offset];
}

u32 vmmap(m_map* map_head) {

    if (map_head == NULL) {
        return E_ERR;
    }

    while (map_head != NULL) {

        printf("%.8x -- %.8x ",
               map_head->v_addr,
               map_head->v_addr + map_head->v_size
        );

        if (map_head->m_prot & PROT_READ) {
            printf("r");
        }
        else {
            printf("-");
        }

        if (map_head->m_prot & PROT_WRITE) {
            printf("w");
        }
        else {
            printf("-");
        }

        if (map_head->m_prot & PROT_EXEC) {
            printf("x");
        }
        else {
            printf("-");
        }

        if (map_head->m_name != NULL) {
            printf(" %s", map_head->m_name);
        }
        else {
            printf("<?>");
        }

        if (map_head->m_file != NULL) {
            printf(" %s\n", map_head->m_file);
        }
        else {
            printf("<...>\n");
        }

        map_head = map_head->m_next;
    }

    return E_OK;
}

u32 deallocate_map(m_map* map_head, u32 v_addr) {

    m_map* t_map = NULL;

    if (map_head == NULL) {
        return E_ERR;
    }

    while (map_head->m_next != NULL) {

        if (v_addr == map_head->v_addr) {
            break;
        }
        t_map = map_head;
        map_head = map_head->m_next;
    }

    if (t_map != NULL) {
        t_map->m_next = map_head->m_next;
    }

    map_head->m_name = NULL;
    map_head->m_file = NULL;
    map_head->v_size = 0x00;
    map_head->a_size = 0x00;
    map_head->m_prot = 0x00;
    map_head->m_ofst = 0x00;
    map_head->m_next = NULL;
    free(map_head->m_buff);	map_head->m_buff = NULL;
    free(map_head); map_head = NULL;
    return E_OK;
}

u32 fini_m_map(m_map * map_head) {

    m_map* t_map = map_head;
    m_map* p_map = NULL;

    if (t_map == NULL) {
        return E_ERR;
    }

    while (t_map != NULL) {

        p_map = t_map;
        t_map = t_map->m_next;

        p_map->m_name = NULL;
        p_map->m_file = NULL;
        p_map->v_size = 0x00;
        p_map->a_size = 0x00;
        p_map->m_prot = 0x00;
        p_map->m_ofst = 0x00;
        p_map->m_name = NULL;
        free(p_map->m_buff); p_map->m_buff = NULL;
        free(p_map); p_map = NULL;
    }

    return E_OK;
}