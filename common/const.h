//
// Created by X3eRo0 on 2/21/2021.
//

#ifndef XVM_CONST_H
#define XVM_CONST_H

#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#define XVM_MAGIC 0x036d7678 // "xvm\x03"
#define XVM_DFLT_EP 0x13371000
#define XVM_DFLT_DP 0x1337F000
#define XVM_DFLT_SP 0xCAFE3ffc
#define XVM_STACK_SIZE 0x3000
#define XVM_HDR_SIZE 0x14

#define E_OK 0
#define E_ERR (-1)

typedef enum {

    PERM_READ = 1,
    PERM_WRITE = 2,
    PERM_EXEC = 4,

} mem_perms;

typedef enum {
    XVM_OP_MOV,
    XVM_OP_MOVB,
    XVM_OP_MOVW,
    XVM_OP_CMOVE,
    XVM_OP_CMOVEW,
    XVM_OP_CMOVEB,
    XVM_OP_CMOVZ,
    XVM_OP_CMOVZW,
    XVM_OP_CMOVZB,
    XVM_OP_CMOVNE,
    XVM_OP_CMOVNEW,
    XVM_OP_CMOVNEB,
    XVM_OP_CMOVNZ,
    XVM_OP_CMOVNZW,
    XVM_OP_CMOVNZB,
    XVM_OP_CMOVA,
    XVM_OP_CMOVAW,
    XVM_OP_CMOVAB,
    XVM_OP_CMOVAE,
    XVM_OP_CMOVAEW,
    XVM_OP_CMOVAEB,
    XVM_OP_CMOVB,
    XVM_OP_CMOVBW,
    XVM_OP_CMOVBB,
    XVM_OP_CMOVBE,
    XVM_OP_CMOVBEW,
    XVM_OP_CMOVBEB,
    XVM_OP_LEA,
    XVM_OP_NOP,
    XVM_OP_HLT,
    XVM_OP_RET,
    XVM_OP_CALL,
    XVM_OP_SYSC,
    XVM_OP_LSU,
    XVM_OP_RSU,
    XVM_OP_ADD,
    XVM_OP_ADDB,
    XVM_OP_ADDW,
    XVM_OP_SUB,
    XVM_OP_SUBB,
    XVM_OP_SUBW,
    XVM_OP_MUL,
    XVM_OP_MULB,
    XVM_OP_MULW,
    XVM_OP_DIV,
    XVM_OP_DIVB,
    XVM_OP_DIVW,
    XVM_OP_XOR,
    XVM_OP_XORB,
    XVM_OP_XORW,
    XVM_OP_AND,
    XVM_OP_ANDB,
    XVM_OP_ANDW,
    XVM_OP_OR,
    XVM_OP_ORB,
    XVM_OP_ORW,
    XVM_OP_NOT,
    XVM_OP_NOTB,
    XVM_OP_NOTW,
    XVM_OP_PUSH,
    XVM_OP_PUSHA,
    XVM_OP_POP,
    XVM_OP_POPA,
    XVM_OP_XCHG,
    XVM_OP_INC,
    XVM_OP_DEC,
    XVM_OP_CMP,
    XVM_OP_CMPB,
    XVM_OP_CMPW,
    XVM_OP_TEST,
    XVM_OP_JMP,
    XVM_OP_JZ,
    XVM_OP_JNZ,
    XVM_OP_JA,
    XVM_OP_JB,
    XVM_OP_JAE,
    XVM_OP_JBE,
    XVM_OP_RJMP,
    XVM_OP_RJZ,
    XVM_OP_RJNZ,
    XVM_OP_RJA,
    XVM_OP_RJB,
    XVM_OP_RJAE,
    XVM_OP_RJBE,
} xvm_opcodes;

#define KNRM "\x1B[0m"
#define KRED "\x1B[31m"
#define KGRN "\x1B[32m"
#define KYEL "\x1B[33m"
#define KBLU "\x1B[34m"
#define KMAG "\x1B[35m"
#define KCYN "\x1B[36m"
#define KWHT "\x1B[37m"

typedef uint8_t u8;
typedef uint16_t u16;
typedef uint32_t u32;
typedef uint64_t u64;

#endif // XVM_CONST_H
