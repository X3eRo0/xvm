//
// Created by X3eRo0 on 9/16/2021.
//
#include "signals.h"

u32 raise_signal(signal_report* err, u32 signal_id, u32 error_addr, u32 error_misc)
{
    if (err != NULL) {
        err->signal_id = signal_id;
        err->error_addr = error_addr;
        err->error_misc = error_misc;
    } else {
        return E_ERR;
    }
    return E_OK;
}
