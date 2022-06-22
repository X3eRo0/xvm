//
// Created by X3eRo0 on 9/16/2021.
//

#ifndef XVM_SIGNALS_H
#define XVM_SIGNALS_H

#include <const.h>

typedef enum singals_t {
    NOSIGNAL,
    XSIGSEGV,
    XSIGTRAP,
    XSIGSTOP,
    XSIGFPE,
    XSIGILL,
} signals;

typedef struct signal_report_t {
    u32 signal_id;
    u32 error_addr;
    u32 error_misc; // usage is dependent on particular signal_report
} signal_report;

u32 raise_signal(signal_report* err, u32 signal_id, u32 error_addr, u32 error_misc);
#endif // XVM_SIGNALS_H
