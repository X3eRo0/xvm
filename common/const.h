//
// Created by X3eRo0 on 2/21/2021.
//

#ifndef XVM_CONST_H
#define XVM_CONST_H

#include <stdint.h>
#include <string.h>
#include  <stdio.h>
#include <stdlib.h>

#define XVM_MAGIC       0x036d7678 // "xvm\x03"
#define XVM_DFLT_EP     0x90000000
#define XVM_HDR_SIZE    0x14

#define E_OK		 0
#define E_ERR		(-1)

typedef enum {

    PERM_READ = 1,
    PERM_WRITE = 2,
    PERM_EXEC = 4,

} mem_perms;


#define KNRM  "\x1B[0m"
#define KRED  "\x1B[31m"
#define KGRN  "\x1B[32m"
#define KYEL  "\x1B[33m"
#define KBLU  "\x1B[34m"
#define KMAG  "\x1B[35m"
#define KCYN  "\x1B[36m"
#define KWHT  "\x1B[37m"

typedef uint8_t  u8;
typedef uint16_t u16;
typedef uint32_t u32;
typedef uint64_t u64;

#endif //XVM_CONST_H
