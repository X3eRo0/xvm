//
// Created by X3eRo0 on 2/28/2021.
//

#include "sections.h"

section_entry* init_section_entry(){
    // initialize section

    section_entry* sec_entry = (section_entry*)malloc(sizeof(section_entry));
    sec_entry->name = NULL;
    sec_entry->size = 0;
    sec_entry->flag = PERM_READ;
    sec_entry->indx = 0;
    sec_entry->buff = NULL;
    sec_entry->next = NULL;

    return sec_entry;
}

u8 read_byte_to_buffer_at_index(section_entry* sec_entry, u32 indx){
    // read byte
    return (u8)*((u8*)&sec_entry->buff[indx]);
}

u16 read_word_to_buffer_at_index(section_entry* sec_entry, u32 indx){
    // read word
    return (u16)*((u16*)&sec_entry->buff[indx]);
}

u32 read_dword_to_buffer_at_index(section_entry* sec_entry, u32 indx){
    // read dword
    return (u32)*((u32*)&sec_entry->buff[indx]);
}

u32 write_section_entry_to_file(section_entry* sec_entry, FILE* file){

    fwrite(sec_entry->name, sizeof(u8), strlen(sec_entry->name) + 1, file);
    fwrite(&sec_entry->size, sizeof(u32), 1, file);
    fwrite(&sec_entry->flag, sizeof(u32), 1, file);
    fwrite(sec_entry->buff, sizeof(u8), sec_entry->indx, file);

    return E_OK;
}

u32 write_byte_to_buffer(section_entry* sec_entry, u8 byte){
    // write byte
    *((u8*)&sec_entry->buff[sec_entry->indx]) = byte;
    sec_entry->indx += sizeof(u8);
    return sizeof(u8);
}

u32 write_word_to_buffer(section_entry* sec_entry, u16 word){
    // write word
    *((u16*)&sec_entry->buff[sec_entry->indx]) = word;
    sec_entry->indx += sizeof(u16);
    return sizeof(u16);
}

u32 write_dword_to_buffer(section_entry* sec_entry, u32 dword){
    // write dword
    *((u32*)&sec_entry->buff[sec_entry->indx]) = dword;
    sec_entry->indx += sizeof(u32);
    return sizeof(u32);
}

u32 memcpy_to_buffer(section_entry* sec_entry, char* buffer, u32 size){

    u32 b_written = 0;
    for (u32 i = 0; i < size; i++){
        b_written += write_byte_to_buffer(sec_entry, buffer[i]);
    }

    return b_written;
}

u32 write_byte_to_buffer_at_index(section_entry* sec_entry, u32 indx, u8 byte){
    // write byte
    *((u8*)&sec_entry->buff[indx]) = byte;
    return sizeof(u8);
}

u32 write_word_to_buffer_at_index(section_entry* sec_entry, u32 indx, u16 word){
    // write word
    *((u16*)&sec_entry->buff[indx]) = word;
    return sizeof(u16);
}

u32 write_dword_to_buffer_at_index(section_entry* sec_entry, u32 indx, u32 dword){
    // write dword
    *((u32*)&sec_entry->buff[indx]) = dword;
    return sizeof(u32);
}

u32 set_section_entry(section_entry* sec_entry, char* name, u32 size, u32 flag){
    // set section entry members

    if (sec_entry->name == NULL){
        sec_entry->name = strdup(name);
    } else {
        sec_entry->name = realloc(sec_entry->name, strlen(name) + 1);
        strncpy(sec_entry->name, name, strlen(name));
    }

    sec_entry->size = size;
    sec_entry->flag = flag;
    sec_entry->indx = 0;
    sec_entry->buff = (char*)realloc(sec_entry->buff, size);
    sec_entry->next = NULL;

    return E_OK;
}

u32 fini_section_entry(section_entry* sec_entry){
    // destroy section structure

    if (sec_entry->buff){
        free(sec_entry->buff); sec_entry->buff = NULL;
    }

    if (sec_entry->name){
        free(sec_entry->name); sec_entry->name = NULL;
    }
    sec_entry->indx = 0;
    sec_entry->flag = 0;
    sec_entry->size = 0;
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

char* add_section(section* sec, char* name, u32 size, u32 flag){
    // add new section to section list

    if (sec == NULL) {
        return NULL;
    }

    if (sec->sections == NULL) {
        sec->sections = init_section_entry();
        sec->n_sections++;
        set_section_entry(sec->sections, name, size, flag);
        return sec->sections->name;
    }

    section_entry* temp = sec->sections;
    section_entry* prev = NULL;

    while (temp != NULL){

        if (!strncmp(temp->name, name, strlen(name))){
            if (temp->size != size){
                temp->size = size;
                temp->buff = realloc(temp->buff, size);
            }
            if (temp->flag != flag){
                temp->flag = flag;
            }
            return temp->name;
        }
        prev = temp;
        temp = temp->next;
    }

    prev->next = init_section_entry();
    sec->n_sections++;
    set_section_entry(prev->next, name, size, flag);

    return prev->next->name;
}

section_entry* find_section_entry_by_name(section* sec, char* name){

    section_entry* temp = sec->sections;
    while (temp != NULL){
        if (!strncmp(temp->name, name, strlen(name))){
            return temp;
        }
        temp = temp->next;
    }

    return NULL;
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
        if (!strncmp(temp->name, name, strlen(name))){
            break;
        }
        temp = temp->next;
    }

    if (temp == NULL){
        return E_ERR;
    }

    switch(write_as_flag){
        case WRITE_AS_BYTE: write_byte_to_buffer(temp, (u8)buffer); return E_OK;
        case WRITE_AS_WORD: write_word_to_buffer(temp, (u16)buffer); return E_OK;
        case WRITE_AS_DWORD: write_dword_to_buffer(temp, (u32)buffer); return E_OK;
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
        if (!strncmp(temp->name, name, strlen(name))){
            break;
        }
        temp = temp->next;
    }
    return memcpy_to_buffer(temp, buffer, size);
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