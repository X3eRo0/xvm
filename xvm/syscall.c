//
// Created by X3eRo0 on 4/18/2021.
//

#include "cpu.h"

// do_syscall($r0, $r1, $r2, $r3)

u32 do_syscall(xvm_cpu* cpu, xvm_bin* bin){

    switch(cpu->regs.r0){

        // read syscall
        case XVM_SYSC_READ:{
            section_entry * temp = find_section_entry_by_addr(bin->x_section, cpu->regs.r2);
            int fd = (int)cpu->regs.r1;
            size_t count = cpu->regs.r3;

            if (cpu->regs.r2 + count > temp->v_addr + temp->v_size){
                count = (temp->v_addr + temp->v_size) - cpu->regs.r2;
            }

            void * buf = get_reference(bin->x_section, cpu->regs.r2, PERM_WRITE);
            cpu->regs.r0 = read(fd, buf, count);
            break;
        }

        // write syscall
        case XVM_SYSC_WRITE: {
            section_entry * temp = find_section_entry_by_addr(bin->x_section, cpu->regs.r2);
            int fd = (int)cpu->regs.r1;
            size_t count = cpu->regs.r3;

            if (cpu->regs.r2 + count > temp->v_addr + temp->v_size){
                count = (temp->v_addr + temp->v_size) - cpu->regs.r2;
            }

            void * buf = get_reference(bin->x_section, cpu->regs.r2, PERM_READ);
            cpu->regs.r0 = write(fd, buf, count);
            break;
        }

        // MAP syscall
        case XVM_SYSC_MAP: {
            // you cannot unmap or map on top of already mapped sections
            section_entry * temp = bin->x_section->sections;
            while (temp != NULL){
                if (temp->v_addr == cpu->regs.r2){
                    cpu->regs.r0 = E_ERR;
                    break;
                }
                temp = temp->next;
            }

            add_section(bin->x_section, NULL, cpu->regs.r1, cpu->regs.r2, cpu->regs.r3);
            cpu->regs.r0 = cpu->regs.r2;

            show_section_info(bin->x_section);

            break;
        }

        case XVM_SYSC_UNMAP: {

            section_entry * temp  = find_section_entry_by_addr(bin->x_section, cpu->regs.r1);
            section_entry * prev  = NULL;
            section_entry * text  = find_section_entry_by_name(bin->x_section, ".text");
            section_entry * data  = find_section_entry_by_name(bin->x_section, ".data");
            section_entry * stack = find_section_entry_by_name(bin->x_section, "stack");

            if ((temp->v_addr == text->v_addr) || (temp->v_addr == data->v_addr) || (temp->v_addr == stack->v_addr)){
                cpu->regs.r0 = E_ERR;
                break;
            }

            temp = bin->x_section->sections;
            while (temp != NULL){
                if (temp->v_addr == cpu->regs.r1){
                    break;
                }
                prev = temp;
                temp = temp->next;
            }

            // if temp is NULL we are deallocating a section which does not exist
            if (temp == NULL){
                return E_ERR;
            }

            if (prev != NULL){
                prev->next = temp->next;
            }

            fini_section_entry(temp); temp = NULL; prev = NULL;
            cpu->regs.r0 = E_OK;

            show_section_info(bin->x_section);

            break;
        }

        case XVM_SYSC_EXEC:{
            char * binary = (char *)get_reference(bin->x_section, cpu->regs.r1, PERM_WRITE);
            cpu->regs.r0 = system(binary);
            break;
        }
    }

    return cpu->regs.r0;
}