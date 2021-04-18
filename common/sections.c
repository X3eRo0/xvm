//
// Created by X3eRo0 on 2/28/2021.
//

#include "sections.h"

section_entry* init_section_entry(){
    // initialize section

    section_entry* sec_entry = (section_entry*)malloc(sizeof(section_entry));
    sec_entry->m_name = NULL;
    sec_entry->v_size = 0;
    sec_entry->v_addr = 0;
    sec_entry->a_size = 0;
    sec_entry->m_flag = PERM_READ;
    sec_entry->m_buff = NULL;
    sec_entry->m_ofst = 0;
    sec_entry->next = NULL;

    return sec_entry;
}

u32* get_reference(section *sec, u32 addr, u8 opt_perm) {
    // read byte
    section_entry * sec_entry = find_section_entry_by_addr(sec, addr);
    if (sec_entry != NULL){
        if (!(sec_entry->m_flag & (PERM_READ | opt_perm))){
            segfault(XVM_INVALID_READ, sec_entry, addr);
        }
        return ((u32*)&sec_entry->m_buff[addr - sec_entry->v_addr]);
    }

    segfault(XVM_INVALID_ADDR, sec_entry, addr);
    return NULL;
}

u8 read_byte(section *sec, u32 addr, u8 opt_perm) {
    // read byte
    section_entry * sec_entry = find_section_entry_by_addr(sec, addr);
    if (sec_entry != NULL){
        if (!(sec_entry->m_flag & (PERM_READ | opt_perm))){
            segfault(XVM_INVALID_READ, sec_entry, addr);
        }
        return (u8)*((u8*)&sec_entry->m_buff[addr - sec_entry->v_addr]);
    }

    segfault(XVM_INVALID_ADDR, sec_entry, addr);
    return E_ERR;
}

u16 read_word(section *sec, u32 addr, u8 opt_perm) {
    // read word
    section_entry * sec_entry = find_section_entry_by_addr(sec, addr);
    if (sec_entry != NULL){
        if (!(sec_entry->m_flag & (PERM_READ | opt_perm))){
            segfault(XVM_INVALID_READ, sec_entry, addr);
        }
        return (u16)*((u16*)&sec_entry->m_buff[addr - sec_entry->v_addr]);
    }

    segfault(XVM_INVALID_ADDR, sec_entry, addr);
    return E_ERR;
}

u32 read_dword(section* sec, u32 addr, u8 opt_perm){
    // read dword
    section_entry * sec_entry = find_section_entry_by_addr(sec, addr);
    if (sec_entry != NULL){
        if (!(sec_entry->m_flag & (PERM_READ | opt_perm))){
            segfault(XVM_INVALID_READ, sec_entry, addr);
        }
        return (u32)*((u32*)&sec_entry->m_buff[addr - sec_entry->v_addr]);
    }

    segfault(XVM_INVALID_ADDR, sec_entry, addr);
    return E_ERR;
}

u32 write_section_entry_to_file(section_entry* sec_entry, FILE* file){

    fwrite(sec_entry->m_name, sizeof(u8), strlen(sec_entry->m_name) + 1, file);
    fwrite(&sec_entry->v_size, sizeof(u32), 1, file);
    fwrite(&sec_entry->v_addr, sizeof(u32), 1, file);
    fwrite(&sec_entry->m_flag, sizeof(u32), 1, file);
    fwrite(&sec_entry->m_ofst, sizeof(u32), 1, file);
    fwrite(sec_entry->m_buff, sizeof(u8), sec_entry->m_ofst, file);

    return E_OK;
}

u32 append_byte(section_entry* sec_entry, u8 byte){
    // write byte
    *((u8*)&sec_entry->m_buff[sec_entry->m_ofst]) = byte;
    sec_entry->m_ofst += sizeof(u8);
    return sizeof(u8);
}

u32 append_word(section_entry* sec_entry, u16 word){
    // write word
    *((u16*)&sec_entry->m_buff[sec_entry->m_ofst]) = word;
    sec_entry->m_ofst += sizeof(u16);
    return sizeof(u16);
}

u32 append_dword(section_entry* sec_entry, u32 dword){
    // write dword
    *((u32*)&sec_entry->m_buff[sec_entry->m_ofst]) = dword;
    sec_entry->m_ofst += sizeof(u32);
    return sizeof(u32);
}

u32 memcpy_to_buffer(section_entry* sec_entry, char* buffer, u32 size){

    u32 b_written = 0;
    for (u32 i = 0; i < size; i++){
        b_written += append_byte(sec_entry, buffer[i]);
    }

    return b_written;
}

u32 write_byte(section *sec, u32 addr, u8 byte) {
    // write byte
    section_entry * sec_entry = find_section_entry_by_addr(sec, addr);
    if (sec_entry != NULL){
        if (!(sec_entry->m_flag & (PERM_WRITE))){
            segfault(XVM_INVALID_WRITE, sec_entry, addr);
        }
        *((u8*)&sec_entry->m_buff[addr - sec_entry->v_addr]) = byte;
        return sizeof(u8);
    }

    segfault(XVM_INVALID_ADDR, sec_entry, addr);
    return E_ERR;
}

u32 write_word(section *sec, u32 addr, u16 word){
    // write word
    section_entry * sec_entry = find_section_entry_by_addr(sec, addr);
    if (sec_entry != NULL){
        if (!(sec_entry->m_flag & (PERM_WRITE))){
            segfault(XVM_INVALID_WRITE, sec_entry, addr);
        }
        *((u16*)&sec_entry->m_buff[addr - sec_entry->v_addr]) = word;
        return sizeof(u16);
    }

    segfault(XVM_INVALID_ADDR, sec_entry, addr);
    return E_ERR;
}

u32 write_dword(section *sec, u32 addr, u32 dword){
    // write dword
    section_entry * sec_entry = find_section_entry_by_addr(sec, addr);
    if (sec_entry != NULL){
        if (!(sec_entry->m_flag & PERM_WRITE)){
            segfault(XVM_INVALID_WRITE, sec_entry, addr);
        }
        *((u32*)&sec_entry->m_buff[addr - sec_entry->v_addr]) = dword;
        return sizeof(u32);
    }
    segfault(XVM_INVALID_ADDR, sec_entry, addr);
    return E_ERR;
}

u32 set_section_entry(section_entry* sec_entry, char* name, u32 size, u32 addr, u32 flag){
    // set section entry members

    if ((size % 0x1000) != 0){
        size = ((size/0x1000) + 1) * 0x1000;
    }

    // FIXME: check if any other section entry already has the same values

    // if size is greater than the limit, adjust the size
    size = size > MAX_ALLOC_SIZE ? MAX_ALLOC_SIZE : size;

    if (sec_entry->m_name == NULL){
        sec_entry->m_name = strdup(name);
    } else {
        sec_entry->m_name = realloc(sec_entry->m_name, strlen(name) + 1);
        strncpy(sec_entry->m_name, name, strlen(name));
    }

    sec_entry->v_size = size;
    sec_entry->v_addr = addr;
    sec_entry->m_flag = flag;
    sec_entry->m_ofst = 0;
    sec_entry->a_size = 0;
    sec_entry->m_buff = (char*)realloc(sec_entry->m_buff, size);
    sec_entry->next = NULL;

    return E_OK;
}

u32 show_section_entry_info(section_entry* sec_entry){

    printf(KGRN "%8s" KNRM, sec_entry->m_name);
    printf(KBLU "        #0x%X" KNRM, sec_entry->v_size);
    printf(KYEL "    ");
    if (sec_entry->m_flag & PERM_READ) {
        printf("r");
    }
    else {
        printf("-");
    }

    if (sec_entry->m_flag & PERM_WRITE) {
        printf("w");
    }
    else {
        printf("-");
    }

    if (sec_entry->m_flag & PERM_EXEC) {
        printf("x");
    }
    else {
        printf("-");
    }
    printf(KNRM "       ");

    printf(KRED "#0x%06X" KNRM, sec_entry->m_ofst);

    printf(KNRM "       ");

    printf(KGRN "#0x%06X\n" KNRM, sec_entry->v_addr);

    return E_OK;
}

u32 fini_section_entry(section_entry* sec_entry){
    // destroy section structure

    // BUG: if we delete the 1st section
    // when it is not the last section in
    // the list, then we basically loose
    // all the sections which were after
    // the 1st section.

    if (sec_entry->next != NULL && !strncmp(sec_entry->m_name, ".text", 5)){
        return E_ERR;
    }

    if (sec_entry->m_buff){
        free(sec_entry->m_buff); sec_entry->m_buff = NULL;
    }

    if (sec_entry->m_name){
        free(sec_entry->m_name); sec_entry->m_name = NULL;
    }

    sec_entry->v_size = 0;
    sec_entry->v_addr = 0;
    sec_entry->a_size = 0;
    sec_entry->m_ofst = 0;
    sec_entry->m_flag = 0;

    sec_entry->next = NULL;
    free(sec_entry); sec_entry = NULL;

    return E_OK;
}

section* init_section(){
    // initialize section list

    section* sec  = (section*)malloc(sizeof(section));
    sec->sections = NULL;
    sec->n_sections = 0;
    return sec;
}

section_entry* add_section(section* sec, char* name, u32 size, u32 addr, u32 flag){
    // add new section to section list

    size = (size % 0x1000) == 0 ? size : (size/0x1000 + 1) * 0x1000;
    size = size > MAX_ALLOC_SIZE ? MAX_ALLOC_SIZE : size;

    if (sec == NULL) {
        return NULL;
    }

    if (sec->sections == NULL) {
        sec->sections = init_section_entry();
        sec->n_sections++;
        set_section_entry(sec->sections, name, size, addr, flag);
        return sec->sections;
    }

    section_entry* temp = sec->sections;
    section_entry* prev = NULL;

    while (temp != NULL){

        if (!strncmp(temp->m_name, name, strlen(name))){
            if (temp->v_size != size){
                temp->v_size = size;
                temp->m_buff = realloc(temp->m_buff, size);
            }
            if (temp->m_flag != flag){
                temp->m_flag = flag;
            }
            if (temp->v_addr != addr){
                temp->v_addr = addr;
            }

            return temp;
        }
        prev = temp;
        temp = temp->next;
    }

    prev->next = init_section_entry();
    sec->n_sections++;
    set_section_entry(prev->next, name, size, addr, flag);

    return prev->next;
}

section_entry* find_section_entry_by_name(section* sec, char* name){

    section_entry* temp = sec->sections;
    while (temp != NULL){
        if (!strncmp(temp->m_name, name, strlen(name))){
            return temp;
        }
        temp = temp->next;
    }

    return NULL;
}

section_entry* find_section_entry_by_addr(section* sec, u32 addr){

    section_entry* temp = sec->sections;
    while (temp != NULL){
        if (addr >= temp->v_addr && addr <= temp->v_addr + temp->v_size){
            return temp;
        }
        temp = temp->next;
    }

    return NULL;
}

u32 show_section_info(section* sec){

    printf("[" KGRN "+" KNRM "] Dumping Section Info\n");
    printf("Section Name      Size    Flags    Size on Disk      v-address\n");

    section_entry* temp = sec->sections;
    while (temp != NULL){
        show_section_entry_info(temp);
        temp = temp->next;
    }

    return E_OK;
}

u32 write_section_to_file(section* sec, FILE* file){

    section_entry* temp = sec->sections;
    while (temp != NULL){
        write_section_entry_to_file(temp, file);
        temp = temp->next;
    }

    return E_OK;
}

u32 write_buffer_to_section_by_name(section* sec, char* name, u32 buffer, u32 write_as_flag){

    if (sec == NULL) {
        return E_ERR;
    }

    if (sec->sections == NULL){
        return E_ERR;
    }

    section_entry* temp = sec->sections;

    while (temp != NULL){
        if (!strncmp(temp->m_name, name, strlen(name))){
            break;
        }
        temp = temp->next;
    }

    if (temp == NULL){
        return E_ERR;
    }

    switch(write_as_flag){
        case WRITE_AS_BYTE:
            append_byte(temp, (u8) buffer); return E_OK;
        case WRITE_AS_WORD:
            append_word(temp, (u16) buffer); return E_OK;
        case WRITE_AS_DWORD:
            append_dword(temp, (u32) buffer); return E_OK;
        default: return E_ERR;
    }
}

u32 write_buffer_to_section_by_addr(section* sec, u32 addr, u32 buffer, u32 write_as_flag){

    if (sec == NULL) {
        return E_ERR;
    }

    if (sec->sections == NULL){
        return E_ERR;
    }

    section_entry* temp = sec->sections;

    while (temp != NULL){
        if (addr >= temp->v_addr && addr <= temp->v_addr + temp->v_size){
            break;
        }
        temp = temp->next;
    }

    if (temp == NULL){
        return E_ERR;
    }

    switch(write_as_flag){
        case WRITE_AS_BYTE:
            append_byte(temp, (u8) buffer); return E_OK;
        case WRITE_AS_WORD:
            append_word(temp, (u16) buffer); return E_OK;
        case WRITE_AS_DWORD:
            append_dword(temp, (u32) buffer); return E_OK;
        default: return E_ERR;
    }
}

u32 memcpy_buffer_to_section_by_name(section* sec, char* name, char* buffer, u32 size){

    if (sec == NULL) {
        return E_ERR;
    }

    if (sec->sections == NULL){
        return E_ERR;
    }

    section_entry* temp = sec->sections;
    while (temp != NULL){
        if (!strncmp(temp->m_name, name, strlen(name))){
            break;
        }
        temp = temp->next;
    }
    return memcpy_to_buffer(temp, buffer, size);
}

u32 memcpy_buffer_to_section_by_addr(section* sec, u32 addr, char* buffer, u32 size){

    if (sec == NULL) {
        return E_ERR;
    }

    if (sec->sections == NULL){
        return E_ERR;
    }

    section_entry* temp = sec->sections;
    while (temp != NULL){
        if (addr >= temp->v_addr && addr <= temp->v_addr + temp->v_size){
            break;
        }
        temp = temp->next;
    }
    return memcpy_to_buffer(temp, buffer, size);
}

u32 reset_address_of_sections(section* sec){
    // reset address member to 0

    section_entry* temp = sec->sections;
    while (temp != NULL){
        temp->a_size = 0;
        temp = temp->next;
    }

    return E_OK;
}

u32 fini_section(section* sec){
    // destroy section list

    section_entry* temp = sec->sections;
    section_entry* prev = NULL;

    while (temp != NULL){
        prev = temp;
        temp = temp->next;
        fini_section_entry(prev);
    }

    sec->sections = NULL;
    free(sec); sec = NULL;
    return E_OK;
}

void segfault(u32 error, section_entry* sec_entry, u32 addr){
    fprintf(stderr, "Segmentation Fault : ");
    if (sec_entry != NULL){
        fprintf(stderr, "%s-%d-0x%x : ", sec_entry->m_name, sec_entry->m_flag, sec_entry->v_addr);
    }
    switch (error) {
        case XVM_INVALID_ADDR: fprintf(stderr, "Invalid Address @ 0x%x\n", addr);break;
        case XVM_INVALID_READ: fprintf(stderr, "Invalid Read @ 0x%x\n", addr);break;
        case XVM_INVALID_WRITE: fprintf(stderr, "Invalid Write @ 0x%x\n", addr);break;
        case XVM_INVALID_EXEC: fprintf(stderr, "Invalid Code @ 0x%x\n", addr);break;
        default: fprintf(stderr, "XVM BRUH MOMENT\n");break;
    }

    exit((int) error);
}