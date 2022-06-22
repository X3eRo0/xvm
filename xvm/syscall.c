//
// Created by X3eRo0 on 4/18/2021.
//

#include "signals.h"
#include <cpu.h>

// do_syscall($r0, $r1, $r2, $r3)

u32 do_syscall(xvm_cpu* cpu, xvm_bin* bin)
{

    switch (cpu->regs.r0) {

    // read
    case XVM_SYSC_READ: {
        section_entry* temp = find_section_entry_by_addr(bin->x_section, cpu->regs.r2);
        if (temp == NULL) {
            raise_signal(bin->x_section->errors, XSIGSEGV, cpu->regs.r2, 0);
            break;
        }
        int fd = (int)cpu->regs.r1;
        size_t count = cpu->regs.r5;

        if (cpu->regs.r2 + count > temp->v_addr + temp->v_size) {
            count = (temp->v_addr + temp->v_size) - cpu->regs.r2;
        }

        void* buf = get_reference(bin->x_section, cpu->regs.r2, PERM_WRITE);
        cpu->regs.r0 = read(fd, buf, count);
        break;
    }

    // write
    case XVM_SYSC_WRITE: {
        section_entry* temp = find_section_entry_by_addr(bin->x_section, cpu->regs.r2);
        if (temp == NULL) {
            raise_signal(bin->x_section->errors, XSIGSEGV, cpu->regs.r2, 0);
            break;
        }
        int fd = (int)cpu->regs.r1;
        size_t count = cpu->regs.r5;

        if (cpu->regs.r2 + count > temp->v_addr + temp->v_size) {
            count = (temp->v_addr + temp->v_size) - cpu->regs.r2;
        }

        void* buf = get_reference(bin->x_section, cpu->regs.r2, PERM_READ);
        cpu->regs.r0 = write(fd, buf, count);
        break;
    }

    // map
    case XVM_SYSC_MAP: {
        // you cannot unmap or map on top of already mapped sections
        section_entry* temp = bin->x_section->sections;
        while (temp != NULL) {
            if (temp->v_addr == cpu->regs.r2) {
                cpu->regs.r0 = E_ERR;
                break;
            }
            temp = temp->next;
        }

        add_section(bin->x_section, NULL, cpu->regs.r1, cpu->regs.r2, cpu->regs.r5);
        cpu->regs.r0 = cpu->regs.r2;

        show_section_info(bin->x_section);

        break;
    }

    // unmap
    case XVM_SYSC_UNMAP: {

        section_entry* temp = find_section_entry_by_addr(bin->x_section, cpu->regs.r1);
        if (temp == NULL) {
            raise_signal(bin->x_section->errors, XSIGSEGV, cpu->regs.r1, 0);
            break;
        }
        section_entry* prev = NULL;
        section_entry* text = find_section_entry_by_name(bin->x_section, ".text");
        section_entry* data = find_section_entry_by_name(bin->x_section, ".data");
        section_entry* stack = find_section_entry_by_name(bin->x_section, "stack");

        if (text == NULL || data == NULL || stack == NULL) {
            raise_signal(bin->x_section->errors, XSIGSEGV, 0, 0);
            break;
        }

        if ((temp->v_addr == text->v_addr) || (temp->v_addr == data->v_addr) || (temp->v_addr == stack->v_addr)) {
            cpu->regs.r0 = E_ERR;
            break;
        }

        temp = bin->x_section->sections;
        while (temp != NULL) {
            if (temp->v_addr == cpu->regs.r1) {
                break;
            }
            prev = temp;
            temp = temp->next;
        }

        // if temp is NULL we are deallocating a section which does not exist
        if (temp == NULL) {
            return E_ERR;
        }

        if (prev != NULL) {
            prev->next = temp->next;
        }

        fini_section_entry(temp);
        temp = NULL;
        prev = NULL;
        cpu->regs.r0 = E_OK;

        show_section_info(bin->x_section);

        break;
    }

    // exec
    case XVM_SYSC_EXEC: {
        char* binary = (char*)get_reference(bin->x_section, cpu->regs.r1, PERM_WRITE);
        cpu->regs.r0 = system(binary);
        break;
    }

    // open
    case XVM_SYSC_OPEN: {
        char* filename = (char*)get_reference(bin->x_section, cpu->regs.r1, PERM_WRITE);
        cpu->regs.r0 = open(filename, (int)cpu->regs.r2);
        break;
    }

    // close
    case XVM_SYSC_CLOSE: {
        cpu->regs.r0 = close((int)cpu->regs.r1);
        break;
    }

    // bind
    case XVM_SYSC_BIND: {
        struct sockaddr_in addr;
        addr.sin_addr.s_addr = htonl(INADDR_ANY);
        addr.sin_port = htons(cpu->regs.r2);
        addr.sin_family = AF_INET;

        cpu->regs.r0 = bind((int)cpu->regs.r1, (struct sockaddr*)&addr, sizeof(struct sockaddr_in));
        break;
    }

    // accept
    case XVM_SYSC_ACCEPT: {

        struct sockaddr_in addr;
        socklen_t addr_size = sizeof(struct sockaddr_in);
        cpu->regs.r0 = (u16)accept((int)cpu->regs.r1, (struct sockaddr*)&addr, &addr_size);
        break;
    }

    // listen
    case XVM_SYSC_LISTEN: {
        cpu->regs.r0 = listen((int)cpu->regs.r1, (int)cpu->regs.r2);
        break;
    }

    // recv
    case XVM_SYSC_RECV: {

        section_entry* temp = find_section_entry_by_addr(bin->x_section, cpu->regs.r2);
        if (temp == NULL) {
            raise_signal(bin->x_section->errors, XSIGSEGV, cpu->regs.r2, 0);
            break;
        }
        int fd = (int)cpu->regs.r1;
        size_t count = cpu->regs.r5;

        if (cpu->regs.r2 + count > temp->v_addr + temp->v_size) {
            count = (temp->v_addr + temp->v_size) - cpu->regs.r2;
        }

        void* buf = get_reference(bin->x_section, cpu->regs.r2, PERM_WRITE);
        cpu->regs.r0 = recv(fd, buf, count, (int)cpu->regs.r4);
        break;
    }

    // send
    case XVM_SYSC_SEND: {
        section_entry* temp = find_section_entry_by_addr(bin->x_section, cpu->regs.r2);
        if (temp == NULL) {
            raise_signal(bin->x_section->errors, XSIGSEGV, cpu->regs.r2, 0);
            break;
        }
        int fd = (int)cpu->regs.r1;
        size_t count = cpu->regs.r5;

        if (cpu->regs.r2 + count > temp->v_addr + temp->v_size) {
            count = (temp->v_addr + temp->v_size) - cpu->regs.r2;
        }

        void* buf = get_reference(bin->x_section, cpu->regs.r2, PERM_READ);
        cpu->regs.r0 = send(fd, buf, count, (int)cpu->regs.r4);
        break;
    }

    // socket
    case XVM_SYSC_SOCKET: {
        int optval = 0;
        cpu->regs.r0 = socket((int)cpu->regs.r1, (int)cpu->regs.r2, (int)cpu->regs.r5);
        setsockopt((int)cpu->regs.r0, SOL_SOCKET, SO_REUSEADDR, &optval, sizeof(optval));
        break;
    }

    // connect
    case XVM_SYSC_CONNECT: {
        struct sockaddr_in server;
        server.sin_addr.s_addr = inet_addr((char*)get_reference(bin->x_section, cpu->regs.r2, PERM_READ));
        server.sin_family = AF_INET;
        server.sin_port = htons(cpu->regs.r5);

        cpu->regs.r0 = connect((int)cpu->regs.r1, (struct sockaddr*)&server, sizeof(struct sockaddr_in));
        break;
    }

    // dup
    case XVM_SYSC_DUP2: {
        cpu->regs.r0 = dup2((int)cpu->regs.r1, (int)cpu->regs.r2);
        break;
    }

    // invalid syscall
    default: {
        cpu->regs.r0 = -1;
        break;
    }
    }

    return cpu->regs.r0;
}
