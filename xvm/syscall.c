//
// Created by X3eRo0 on 4/18/2021.
//

#include "cpu.h"

// do_syscall($r0, $r1, $r2, $r3)

u32 do_syscall(xvm_cpu* cpu, xvm_bin* bin){

    switch(cpu->regs.r0){

        // read do_syscall
        case XVM_SYSC_READ:{
            section_entry * temp = find_section_entry_by_addr(bin->x_section, cpu->regs.r2);
            int fd = (int)cpu->regs.r0;
            size_t count = cpu->regs.r3;

            if (cpu->regs.r2 + count > temp->v_addr + temp->v_size){
                count = (temp->v_addr + temp->v_size) - cpu->regs.r2;
            }

            void * buf = get_reference(bin->x_section, cpu->regs.r2, PERM_WRITE);
            cpu->regs.r0 = read(fd, buf, count);
            break;
        }

        // write do_syscall
        case XVM_SYSC_WRITE: {
            section_entry * temp = find_section_entry_by_addr(bin->x_section, cpu->regs.r2);
            int fd = (int)cpu->regs.r0;
            size_t count = cpu->regs.r3;

            if (cpu->regs.r2 + count > temp->v_addr + temp->v_size){
                count = (temp->v_addr + temp->v_size) - cpu->regs.r2;
            }

            void * buf = get_reference(bin->x_section, cpu->regs.r2, PERM_READ);
            cpu->regs.r0 = write(fd, buf, count);
            break;
        }
    }

    return cpu->regs.r0;
}