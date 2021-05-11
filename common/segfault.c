//
// Created by X3eRo0 on 5/1/2021.
//
#include "sections.h"

void segfault(u32 error, section_entry* sec_entry, u32 addr){
    fprintf(stderr, "[" KRED "-" KNRM "] Segmentation Fault : ");
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