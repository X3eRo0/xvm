//
// Created by X3eRo0 on 2/28/2021.
//

#include <sections.h>
#include <signals.h>

section_entry* init_section_entry()
{
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

u32* get_reference(section* sec, u32 addr, u8 opt_perm)
{
    // read byte
    section_entry* sec_entry = find_section_entry_by_addr(sec, addr);
    if (sec_entry != NULL) {
        if (!(sec_entry->m_flag & PERM_READ) || !(sec_entry->m_flag & opt_perm)) {
            raise_signal(sec->errors, XSIGSEGV, addr, 0);
        }
        return ((u32*)&sec_entry->m_buff[addr - sec_entry->v_addr]);
    }

    raise_signal(sec->errors, XSIGSEGV, addr, 0);
    return NULL;
}

u8 read_byte(section* sec, u32 addr, u8 opt_perm)
{
    // read byte
    section_entry* sec_entry = find_section_entry_by_addr(sec, addr);
    if (sec_entry != NULL) {
        if (!(sec_entry->m_flag & PERM_READ) || !(sec_entry->m_flag & opt_perm)) {
            raise_signal(sec->errors, XSIGSEGV, addr, 0);
        }
        return (u8) * ((u8*)&sec_entry->m_buff[addr - sec_entry->v_addr]);
    }

    raise_signal(sec->errors, XSIGSEGV, addr, 0);
    return E_ERR;
}

u16 read_word(section* sec, u32 addr, u8 opt_perm)
{
    // read word
    section_entry* sec_entry = find_section_entry_by_addr(sec, addr);
    if (sec_entry != NULL) {
        if (!(sec_entry->m_flag & PERM_READ) || !(sec_entry->m_flag & opt_perm)) {
            raise_signal(sec->errors, XSIGSEGV, addr, 0);
        }
        return (u16) * ((u16*)&sec_entry->m_buff[addr - sec_entry->v_addr]);
    }

    raise_signal(sec->errors, XSIGSEGV, addr, 0);
    return E_ERR;
}

u32 read_dword(section* sec, u32 addr, u8 opt_perm)
{
    // read dword
    section_entry* sec_entry = find_section_entry_by_addr(sec, addr);
    if (sec_entry != NULL) {
        if (!(sec_entry->m_flag & PERM_READ) || !(sec_entry->m_flag & opt_perm)) {
            raise_signal(sec->errors, XSIGSEGV, addr, 0);
        }
        return (u32) * ((u32*)&sec_entry->m_buff[addr - sec_entry->v_addr]);
    }

    raise_signal(sec->errors, XSIGSEGV, addr, 0);
    return E_ERR;
}

u32 write_section_entry_to_file(section_entry* sec_entry, FILE* file)
{

    fwrite("\xEF\xBE\xAD\xDE", sizeof(u32), 1, file);
    fwrite(sec_entry->m_name, sizeof(u8), strlen(sec_entry->m_name) + 1, file);
    fwrite(&sec_entry->v_size, sizeof(u32), 1, file);
    fwrite(&sec_entry->v_addr, sizeof(u32), 1, file);
    fwrite(&sec_entry->m_flag, sizeof(u32), 1, file);
    fwrite(&sec_entry->m_ofst, sizeof(u32), 1, file);
    fwrite(sec_entry->m_buff, sizeof(u8), sec_entry->m_ofst, file);

    return E_OK;
}

u32 write_raw_section_entry_to_file(section_entry* sec_entry, FILE* file)
{
    fwrite(sec_entry->m_buff, sizeof(u8), sec_entry->m_ofst, file);
    return E_OK;
}

u32 append_byte(section* sec, section_entry* sec_entry, u8 byte)
{
    // write byte
    if (sec_entry->m_ofst >= sec_entry->v_size) {
        raise_signal(sec->errors, XSIGSEGV, sec_entry->v_addr + sec_entry->m_ofst, 0);
    }
    *((u8*)&sec_entry->m_buff[sec_entry->m_ofst]) = byte;
    sec_entry->m_ofst += sizeof(u8);
    return sizeof(u8);
}

u32 append_word(section* sec, section_entry* sec_entry, u16 word)
{
    // write word
    if (sec_entry->m_ofst >= sec_entry->v_size) {
        raise_signal(sec->errors, XSIGSEGV, sec_entry->v_addr + sec_entry->m_ofst, 0);
    }
    *((u16*)&sec_entry->m_buff[sec_entry->m_ofst]) = word;
    sec_entry->m_ofst += sizeof(u16);
    return sizeof(u16);
}

u32 append_dword(section* sec, section_entry* sec_entry, u32 dword)
{
    // write dword
    if (sec_entry->m_ofst >= sec_entry->v_size) {
        raise_signal(sec->errors, XSIGSEGV, sec_entry->v_addr + sec_entry->m_ofst, 0);
    }

    *((u32*)&sec_entry->m_buff[sec_entry->m_ofst]) = dword;
    sec_entry->m_ofst += sizeof(u32);
    return sizeof(u32);
}

u32 memcpy_to_buffer(section* sec, section_entry* sec_entry, char* buffer, u32 size)
{

    u32 b_written = 0;
    for (u32 i = 0; i < size; i++) {
        b_written += append_byte(sec, sec_entry, buffer[i]);
    }

    return b_written;
}

u32 write_byte(section* sec, u32 addr, u8 byte)
{
    // write byte
    section_entry* sec_entry = find_section_entry_by_addr(sec, addr);
    if (sec_entry != NULL) {
        if (!(sec_entry->m_flag & (PERM_WRITE))) {
            raise_signal(sec->errors, XSIGSEGV, addr, 0);
        }
        *((u8*)&sec_entry->m_buff[addr - sec_entry->v_addr]) = byte;
        return sizeof(u8);
    }

    raise_signal(sec->errors, XSIGSEGV, addr, 0);
    return E_ERR;
}

u32 write_word(section* sec, u32 addr, u16 word)
{
    // write word
    section_entry* sec_entry = find_section_entry_by_addr(sec, addr);
    if (sec_entry != NULL) {
        if (!(sec_entry->m_flag & (PERM_WRITE))) {
            raise_signal(sec->errors, XSIGSEGV, addr, 0);
        }
        *((u16*)&sec_entry->m_buff[addr - sec_entry->v_addr]) = word;
        return sizeof(u16);
    }

    raise_signal(sec->errors, XSIGSEGV, addr, 0);
    return E_ERR;
}

u32 write_dword(section* sec, u32 addr, u32 dword)
{
    // write dword
    section_entry* sec_entry = find_section_entry_by_addr(sec, addr);
    if (sec_entry != NULL) {
        if (!(sec_entry->m_flag & PERM_WRITE)) {
            raise_signal(sec->errors, XSIGSEGV, addr, 0);
        }
        *((u32*)&sec_entry->m_buff[addr - sec_entry->v_addr]) = dword;
        return sizeof(u32);
    }
    raise_signal(sec->errors, XSIGSEGV, addr, 0);
    return E_ERR;
}

u32 get_byte(section* sec, u32 addr, u8* byte)
{
    // get byte regardless of perms
    *byte = 0;
    section_entry* sec_entry = find_section_entry_by_addr(sec, addr);
    if (sec_entry != NULL) {
        *byte = (u8) * ((u8*)&sec_entry->m_buff[addr - sec_entry->v_addr]);
        return E_OK;
    }
    return E_ERR;
}

u32 get_word(section* sec, u32 addr, u16* word)
{
    // get byte regardless of perms
    *word = 0;
    section_entry* sec_entry = find_section_entry_by_addr(sec, addr);
    if (sec_entry != NULL) {
        *word = (u16) * ((u16*)&sec_entry->m_buff[addr - sec_entry->v_addr]);
        return E_OK;
    }
    return E_ERR;
}

u32 get_dword(section* sec, u32 addr, u32* dword)
{
    // get byte regardless of perms
    *dword = 0;
    section_entry* sec_entry = find_section_entry_by_addr(sec, addr);
    if (sec_entry != NULL) {
        *dword = (u32) * ((u32*)&sec_entry->m_buff[addr - sec_entry->v_addr]);
        return E_OK;
    }
    return E_ERR;
}

u32 set_byte(section* sec, u32 addr, u8 byte)
{
    // set byte regardless of perms
    section_entry* sec_entry = find_section_entry_by_addr(sec, addr);
    if (sec_entry != NULL) {
        *((u8*)&sec_entry->m_buff[addr - sec_entry->v_addr]) = byte;
        return E_OK;
    }
    return E_ERR;
}

u32 set_word(section* sec, u32 addr, u16 word)
{
    // set byte regardless of perms
    section_entry* sec_entry = find_section_entry_by_addr(sec, addr);
    if (sec_entry != NULL) {
        *((u16*)&sec_entry->m_buff[addr - sec_entry->v_addr]) = word;
        return E_OK;
    }
    return E_ERR;
}

u32 set_dword(section* sec, u32 addr, u32 dword)
{
    // get byte regardless of perms
    section_entry* sec_entry = find_section_entry_by_addr(sec, addr);
    if (sec_entry != NULL) {
        *((u32*)&sec_entry->m_buff[addr - sec_entry->v_addr]) = dword;
        return E_OK;
    }
    return E_ERR;
}

u32 set_section_entry(section_entry* sec_entry, char* name, u32 size, u32 addr, u32 flag)
{
    // set section entry members

    if ((size % 0x1000) != 0) {
        size = ((size / 0x1000) + 1) * 0x1000;
    }

    // FIXME: check if any other section entry already has the same values

    // if size is greater than the limit, adjust the size
    size = size > MAX_ALLOC_SIZE ? MAX_ALLOC_SIZE : size;

    if (name != NULL) {
        if (sec_entry->m_name == NULL) {
            sec_entry->m_name = strdup(name);
        } else {
            sec_entry->m_name = realloc(sec_entry->m_name, MAX_NAME_SIZE);
            strncpy(sec_entry->m_name, name, MAX_NAME_SIZE);
        }
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

u32 show_section_entry_info(section_entry* sec_entry)
{

    printf(KGRN "%8s" KNRM, sec_entry->m_name);
    printf(KBLU "        #0x%X" KNRM, sec_entry->v_size);
    printf(KYEL "    ");
    if (sec_entry->m_flag & PERM_READ) {
        printf("r");
    } else {
        printf("-");
    }

    if (sec_entry->m_flag & PERM_WRITE) {
        printf("w");
    } else {
        printf("-");
    }

    if (sec_entry->m_flag & PERM_EXEC) {
        printf("x");
    } else {
        printf("-");
    }
    printf(KNRM "       ");

    printf(KRED "#0x%06X" KNRM, sec_entry->m_ofst);

    printf(KNRM "       ");

    printf(KGRN "#0x%08X" KNRM " - " KGRN "#0x%08X\n" KNRM, sec_entry->v_addr, sec_entry->v_addr + sec_entry->v_size);

    return E_OK;
}

u32 fini_section_entry(section_entry* sec_entry)
{
    // destroy section structure

    if (sec_entry->m_buff) {
        free(sec_entry->m_buff);
        sec_entry->m_buff = NULL;
    }

    if (sec_entry->m_name) {
        free(sec_entry->m_name);
        sec_entry->m_name = NULL;
    }

    sec_entry->v_size = 0;
    sec_entry->v_addr = 0;
    sec_entry->a_size = 0;
    sec_entry->m_ofst = 0;
    sec_entry->m_flag = 0;

    sec_entry->next = NULL;
    free(sec_entry);
    sec_entry = NULL;

    return E_OK;
}

section* init_section()
{
    // initialize section list

    section* sec = (section*)calloc(1, sizeof(section));
    sec->sections = NULL;
    sec->n_sections = 0;
    sec->errors = (signal_report*)calloc(1, sizeof(signal_report));
    return sec;
}

section_entry* add_section(section* sec, char* name, u32 size, u32 addr, u32 flag)
{
    // add new section to section list

    size = (size % 0x1000) == 0 ? size : (size / 0x1000 + 1) * 0x1000;
    size = size > MAX_ALLOC_SIZE ? MAX_ALLOC_SIZE : size;
    size = size == 0 ? 0x1000 : size;

    if ((u64)addr + (u64)size > 0x100000000) {
        return NULL;
    }

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

    // if new section's v_addr is smaller than the head of the list then change
    // the head of the list

    if (addr + size < temp->v_addr) {

        // this check ensures that the first section in the list is always
        // at the lowest address possible

        section_entry* new = init_section_entry();
        sec->n_sections++;
        set_section_entry(new, name, size, addr, flag);
        new->next = sec->sections;
        sec->sections = new;
        return new;
    }

    while (temp != NULL) {

        if (addr >= temp->v_addr && addr < section_end(temp)) {
            if (temp->v_size != size && temp->next != NULL && temp->v_addr + size < temp->next->v_addr) {
                temp->v_size = size;
                temp->m_buff = realloc(temp->m_buff, size);
            }
            if (temp->m_flag != flag) {
                temp->m_flag = flag;
            }

            return temp;
        }

        if (prev != NULL) {
            if ((section_end(prev) < addr) && (addr + size < temp->v_addr)) {
                // insert the new section here
                break;
            }
        }

        prev = temp;
        temp = temp->next;
    }

    prev->next = init_section_entry();
    sec->n_sections++;
    set_section_entry(prev->next, name, size, addr, flag);
    prev->next->next = temp;

    return prev->next;
}

section_entry* find_section_entry_by_name(section* sec, char* name)
{

    section_entry* temp = sec->sections;
    while (temp != NULL) {
        if (temp->m_name != NULL && !strncmp(temp->m_name, name, strlen(name))) {
            return temp;
        }
        temp = temp->next;
    }

    return NULL;
}

section_entry* find_section_entry_by_addr(section* sec, u32 addr)
{

    section_entry* temp = sec->sections;
    while (temp != NULL) {
        if (addr >= temp->v_addr && addr < temp->v_addr + temp->v_size) {
            return temp;
        }
        temp = temp->next;
    }
    return NULL;
}

u32 show_section_info(section* sec)
{

    // FIXME : COMMENT FOR RELEASE
    printf("[" KGRN "+" KNRM "] Dumping Section Info\n");
    printf("Section Name      Size    Flags    Size on Disk      v-address\n");

    section_entry* temp = sec->sections;
    while (temp != NULL) {
        show_section_entry_info(temp);
        temp = temp->next;
    }
    return E_OK;
}

u32 write_section_to_file(section* sec, FILE* file)
{

    section_entry* temp = sec->sections;
    while (temp != NULL) {
        write_section_entry_to_file(temp, file);
        temp = temp->next;
    }

    return E_OK;
}

u32 write_raw_section_to_file(section* sec, FILE* file)
{
    section_entry* temp = sec->sections;
    while (temp != NULL) {
        write_raw_section_entry_to_file(temp, file);
        temp = temp->next;
    }

    return E_OK;
}

u32 write_buffer_to_section_by_name(section* sec, char* name, u32 buffer, u32 write_as_flag)
{

    if (sec == NULL) {
        return E_ERR;
    }

    if (sec->sections == NULL) {
        return E_ERR;
    }

    section_entry* temp = sec->sections;

    while (temp != NULL) {
        if (!strncmp(temp->m_name, name, strlen(name))) {
            break;
        }
        temp = temp->next;
    }

    if (temp == NULL) {
        return E_ERR;
    }

    switch (write_as_flag) {
    case WRITE_AS_BYTE:
        append_byte(sec, temp, (u8)buffer);
        return E_OK;
    case WRITE_AS_WORD:
        append_word(sec, temp, (u16)buffer);
        return E_OK;
    case WRITE_AS_DWORD:
        append_dword(sec, temp, (u32)buffer);
        return E_OK;
    default:
        return E_ERR;
    }
}

u32 write_buffer_to_section_by_addr(section* sec, u32 addr, u32 buffer, u32 write_as_flag)
{

    if (sec == NULL) {
        return E_ERR;
    }

    if (sec->sections == NULL) {
        return E_ERR;
    }

    section_entry* temp = find_section_entry_by_addr(sec, addr);

    if (temp == NULL) {
        return E_ERR;
    }

    switch (write_as_flag) {
    case WRITE_AS_BYTE:
        append_byte(sec, temp, (u8)buffer);
        return E_OK;
    case WRITE_AS_WORD:
        append_word(sec, temp, (u16)buffer);
        return E_OK;
    case WRITE_AS_DWORD:
        append_dword(sec, temp, (u32)buffer);
        return E_OK;
    default:
        return E_ERR;
    }
}

u32 memcpy_buffer_to_section_by_name(section* sec, char* name, char* buffer, u32 size)
{

    if (sec == NULL) {
        return E_ERR;
    }

    if (sec->sections == NULL) {
        return E_ERR;
    }

    section_entry* temp = sec->sections;
    while (temp != NULL) {
        if (!strncmp(temp->m_name, name, strlen(name))) {
            break;
        }
        temp = temp->next;
    }
    return memcpy_to_buffer(sec, temp, buffer, size);
}

u32 memcpy_buffer_to_section_by_addr(section* sec, u32 addr, char* buffer, u32 size)
{

    if (sec == NULL) {
        return E_ERR;
    }

    if (sec->sections == NULL) {
        return E_ERR;
    }

    section_entry* temp = sec->sections;
    while (temp != NULL) {
        if (addr >= temp->v_addr && addr < temp->v_addr + temp->v_size) {
            break;
        }
        temp = temp->next;
    }
    return memcpy_to_buffer(sec, temp, buffer, size);
}

u32 reset_address_of_sections(section* sec)
{
    // reset address member to 0

    section_entry* temp = sec->sections;
    while (temp != NULL) {
        temp->a_size = 0;
        temp = temp->next;
    }

    return E_OK;
}

u32 fini_section(section* sec)
{
    // destroy section list

    section_entry* temp = sec->sections;
    section_entry* prev = NULL;

    while (temp != NULL) {
        prev = temp;
        temp = temp->next;
        fini_section_entry(prev);
    }

    free(sec->errors); sec->errors = NULL;
    sec->sections = NULL;
    free(sec);
    sec = NULL;
    return E_OK;
}
