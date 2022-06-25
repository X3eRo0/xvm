#ifndef BREAKPOINTS_H
#define BREAKPOINTS_H

#include <symbols.h>
#include <const.h>
#include <sections.h>

#define MAX_BREAKPOINTS 256

typedef struct breaklist_t {
    u32 address;
    u32 id;
    u16 op;
    u8 enabled;
    u8 patched;
    struct breaklist_t* next;
} breaklist;

// breakpoint functions
breaklist* init_breakpoint();
u32 add_breakpoint(breaklist* head, section* sec, u32 address);
u32 patch_breakpoint_t(breaklist* head, section* sec);
u32 patch_breakpoint_by_id(breaklist* head, section* sec, u32 id);
u32 patch_breakpoint_by_addr(breaklist* head, section* sec, u32 address);
u32 patch_breakpoints(breaklist* head, section* sec);
u32 unpatch_breakpoint_t(breaklist* head, section* sec);
u32 unpatch_breakpoint_by_id(breaklist* head, section* sec, u32 id);
u32 unpatch_breakpoint_by_addr(breaklist* head, section* sec, u32 address);
u32 unpatch_breakpoints(breaklist* head, section* sec);
u32 enable_breakpoint_t(breaklist* head, section* sec);
u32 enable_breakpoint_by_addr(breaklist* head, section* sec, u32 address);
u32 enable_breakpoint_by_id(breaklist* head, section* sec, u32 id);
u32 enable_breakpoints(breaklist* head, section* sec);
u32 disable_breakpoint_t(breaklist* head, section* sec);
u32 disable_breakpoint_by_addr(breaklist* head, section* sec, u32 address);
u32 disable_breakpoint_by_id(breaklist* head, section* sec, u32 id);
u32 disable_breakpoints(breaklist* head, section* sec);
u32 delete_breakpoint_t(breaklist* head, section* sec);
u32 delete_breakpoints(breaklist* head, section* sec);
u32 delete_breakpoint_by_addr(breaklist* head, section* sec, u32 address);
u32 delete_breakpoint_by_id(breaklist* head, section* sec, u32 id);
u32 show_breakpoint_t(breaklist* head, symtab* syms);
u32 show_breakpoints(breaklist* head, symtab* syms);
#endif // !BREAKPOINTS_H
