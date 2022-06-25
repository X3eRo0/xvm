//
// Created by X3eRo0 on 2/28/2021.
//

#ifndef XVM_SECTIONS_H
#define XVM_SECTIONS_H

#include <const.h>
#include <signals.h>

#define WRITE_AS_BYTE 0
#define WRITE_AS_WORD 1
#define WRITE_AS_DWORD 2

#define MAX_ALLOC_SIZE 0x10000
#define MAX_NAME_SIZE 0x20
#define section_end(sec) (((sec)->v_addr) + ((sec)->v_size))

typedef struct section_entry_t {
    char* m_name; // name of section
    char* m_buff; // byte buffer for this section
    u32 v_size; // virtual size
    u32 v_addr;
    u32 a_size; // actual size
    u32 m_flag; // flags
    u32 m_ofst; // index for buffer
    struct section_entry_t* next; // next section

} section_entry;

typedef struct section_t {
    section_entry* sections;
    u32 n_sections;
    signal_report* errors;
} section;

section_entry* init_section_entry();
u32 set_section_entry(section_entry* sec_entry, char* name, u32 size, u32 addr, u32 flag);

u32 write_raw_section_to_file(section* sec, FILE* file);
u32 write_section_entry_to_file(section_entry* sec_entry, FILE* file);
u32 write_raw_section_entry_to_file(section_entry* sec_entry, FILE* file);
u32 append_byte(section* sec, section_entry* sec_entry, u8 byte);
u32 append_word(section* sec, section_entry* sec_entry, u16 word);
u32 append_dword(section* sec, section_entry* sec_entry, u32 dword);
u32 memcpy_to_buffer(section* sec, section_entry* sec_entry, char* buffer, u32 size);
u32 show_section_entry_info(section_entry* sec_entry);
u32 fini_section_entry(section_entry* sec_entry);
section* init_section();
section_entry* find_section_entry_by_name(section* sec, char* name);
section_entry* find_section_entry_by_addr(section* sec, u32 addr);
u32* get_reference(section* sec, u32 addr, u8 opt_perm);
u8 read_byte(section* sec, u32 addr, u8 opt_perm);
u16 read_word(section* sec, u32 addr, u8 opt_perm);
u32 read_dword(section* sec, u32 addr, u8 opt_perm);
u32 get_byte(section* sec, u32 addr, u8* byte);
u32 get_word(section* sec, u32 addr, u16* word);
u32 get_dword(section* sec, u32 addr, u32* dword);
u32 set_byte(section* sec, u32 addr, u8 byte);
u32 set_word(section* sec, u32 addr, u16 word);
u32 set_dword(section* sec, u32 addr, u32 dword);
u32 write_byte(section* sec, u32 addr, u8 byte);
u32 write_word(section* sec, u32 addr, u16 word);
u32 write_dword(section* sec, u32 addr, u32 dword);
u32 write_section_to_file(section* sec, FILE* file);
u32 write_buffer_to_section_by_addr(section* sec, u32 addr, u32 buffer, u32 write_as_flag);
u32 write_buffer_to_section_by_name(section* sec, char* name, u32 buffer, u32 write_as_flag);
u32 memcpy_buffer_to_section_by_name(section* sec, char* name, char* buffer, u32 size);
u32 memcpy_buffer_to_section_by_addr(section* sec, u32 addr, char* buffer, u32 size);
section_entry* add_section(section* sec, char* name, u32 size, u32 addr, u32 flag);
u32 show_section_info(section* sec);
u32 reset_address_of_sections(section* sec);
u32 fini_section(section* sec);
#endif // XVM_SECTIONS_H
