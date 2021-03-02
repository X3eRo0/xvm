//
// Created by X3eRo0 on 2/28/2021.
//

#ifndef XVM_SECTIONS_H
#define XVM_SECTIONS_H

#include "const.h"

#define WRITE_AS_BYTE 0
#define WRITE_AS_WORD 1
#define WRITE_AS_DWORD 2

typedef struct section_entry_t {
    char*   name;                   // name of section
    u32     size;                   // size
    u32     flag;                   // flags
    u32     indx;                   // index for buffer
    char*   buff;                   // byte buffer for this section
    struct section_entry_t* next;   // next section

} section_entry;

typedef struct section_t {
    section_entry * sections;
    u32 n_sections;
} section;

section_entry*      init_section_entry();
u32                 set_section_entry(section_entry* sec_entry, char* name, u32 size, u32 flag);
u8                  read_byte_to_buffer(section_entry* sec_entry);
u16                 read_word_to_buffer(section_entry* sec_entry);
u32                 read_dword_to_buffer(section_entry* sec_entry);
u32                 write_section_entry_to_file(section_entry* sec_entry, FILE* file);
u32                 write_byte_to_buffer(section_entry* sec_entry, u8 byte);
u32                 write_word_to_buffer(section_entry* sec_entry, u16 word);
u32                 write_dword_to_buffer(section_entry* sec_entry, u32 dword);
u32                 memcpy_to_buffer(section_entry* sec_entry, char* buffer, u32 size);
u32                 write_byte_to_buffer_at_index(section_entry* sec_entry, u32 index, u8 byte);
u32                 write_word_to_buffer_at_index(section_entry* sec_entry, u32 index, u16 word);
u32                 write_dword_to_buffer_at_index(section_entry* sec_entry, u32 index, u32 dword);
u32                 fini_section_entry(section_entry* sec_entry);
section*            init_section();
section_entry*      find_section_entry_by_name(section* sec, char* name);
u32                 write_section_to_file(section* sec, FILE* file);
u32                 write_buffer_to_section_by_name(section* sec, char* name, u32 buffer, u32 write_as_flag);
u32                 memcpy_buffer_to_section_by_name(section* sec, char* name, char* buffer, u32 size);
char*               add_section(section* sec, char* name, u32 size, u32 flag);
u32                 fini_section(section* sec);

#endif //XVM_SECTIONS_H
