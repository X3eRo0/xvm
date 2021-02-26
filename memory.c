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
        map_head->m_buff = (char*)malloc(map_head->v_size);
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
    t_map->v_size = v_size;
    t_map->a_size = a_size;
    t_map->v_addr = v_addr;
    t_map->m_prot = prot;
    t_map->m_ofst = ofst;
    t_map->m_next = NULL;

    return t_map;
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