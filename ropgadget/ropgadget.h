//
// Created by X3eRo0 on 6/20/2022.
//

#ifndef XVM_ROPGADGET_H
#define XVM_ROPGADGET_H

#include <loader.h>
#include <const.h>
#include <xasm.h>

void xvm_find_ropgadgets(xvm_bin* bin, section_entry* sec, u32 depth);

#endif
