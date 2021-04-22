//
// Created by X3eRo0 on 4/15/2021.
//

#include <unistd.h>
#include "../common/const.h"
#include "../common/loader.h"

#ifndef XVM_CPU_H
#define XVM_CPU_H

#define get_mode1(mode) (((mode) >> 0x0) & 0xf)
#define get_mode2(mode) (((mode) >> 0x4) & 0xf)


typedef enum xvm_opcd_t {
    XVM_OP_MOV,
    XVM_OP_MOVB,
    XVM_OP_NOP,
    XVM_OP_HLT,
    XVM_OP_RET,
    XVM_OP_CALL,
    XVM_OP_SYSC,
    XVM_OP_ADD,
    XVM_OP_SUB,
    XVM_OP_MUL,
    XVM_OP_DIV,
    XVM_OP_XOR,
    XVM_OP_AND,
    XVM_OP_OR,
    XVM_OP_NOT,
    XVM_OP_PUSH,
    XVM_OP_POP,
    XVM_OP_XCHG,
    XVM_OP_INC,
    XVM_OP_DEC,
    XVM_OP_CMP,
    XVM_OP_JMP,
    XVM_OP_JZ,
    XVM_OP_JNZ,
    XVM_OP_JA,
    XVM_OP_JB,
} xvm_opcodes;

typedef enum xvm_sysc_t {
    XVM_SYSC_READ,
    XVM_SYSC_WRITE,
    XVM_SYSC_MAP,
    XVM_SYSC_UNMAP,
    XVM_SYSC_EXEC,
} xvm_syscalls;

typedef struct xvm_reg_t {
    u32 r0;
    u32 r1;
    u32 r2;
    u32 r3;
    u32 r4;
    u32 pc;
    u32 bp;
    u32 sp;
} xvm_reg;

typedef enum {
    r0,
    r1,
    r2,
    r3,
    r4,
    pc,
    bp,
    sp,
} xvm_reg_ids;

typedef enum {
    XVM_ZF, // zero flag
    XVM_CF, // carry flag
    XVM_RF, // CPU Halt status
} xvm_flag_pos;

typedef enum {
    XVM_NARG = 0,
    XVM_REGD = 1,
    XVM_IMMD = 2,
    XVM_PTRD = 4,
} xvm_modes;

typedef struct xvm_instr_t
{
    u8 opcode;
    u32 (*instr)(u8 mode, u8 arg1, u8 arg2, u32 imm1, u32 imm2);
} xvm_instr;


typedef struct xvm_flags_t {
    u8 flags;
} xvm_flags;

typedef struct xvm_cpu_t {
    xvm_reg    regs;
    xvm_flags flags;
} xvm_cpu;

void reset_reg(xvm_reg * regs);
xvm_cpu * init_xvm_cpu();
u8 get_RF(xvm_cpu* cpu);
u8 get_CF(xvm_cpu* cpu);
u8 get_ZF(xvm_cpu* cpu);
u8 set_RF(xvm_cpu* cpu, u8 bit);
u8 set_CF(xvm_cpu* cpu, u8 bit);
u8 set_ZF(xvm_cpu* cpu, u8 bit);
u8 get_argument(xvm_cpu* cpu, xvm_bin* bin, u8 mode, u32** arg1, u32** arg2, u8 narg);
u32 *get_register(xvm_cpu* cpu, u8 reg_id);
u32  do_execute(xvm_cpu* cpu, xvm_bin* bin);
u32  do_syscall(xvm_cpu* cpu, xvm_bin* bin);
void fde_cpu(xvm_cpu* cpu, xvm_bin* bin);
void update_flags(xvm_cpu * cpu, u32 res);
void fini_xvm_cpu(xvm_cpu * cpu);

/* xvm instructions
u32 xvm_op_hlt(u8 mode, u8 arg1, u8 arg2, u32 imm1, u32 imm2);
---------------- */
#endif //XVM_CPU_H
