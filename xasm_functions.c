//
// Created by X3eRo0 on 2/23/2021.
//

#include "xasm.h"

xasm* init_xasm(){

    // initialize xvm assembler

    xasm* t_xasm = (xasm*) malloc(sizeof(xasm));
    t_xasm->ifile = NULL;
    t_xasm->ofile = NULL;
    t_xasm->symtab = init_symtab();

    return t_xasm;

}

u32 open_ifile(xasm* xasm, char * file){

    // open input file

    xasm->ifile = fopen(file, "rb");
    return E_OK;

}

u32 open_ofile(xasm* xasm, char * file){

    // open output file

    xasm->ofile = fopen(file, "rb");
    return E_OK;

}

u32 close_ifile(xasm* xasm){

    // close input file

    fclose(xasm->ifile);
    return E_OK;

}

u32 close_ofile(xasm* xasm){

    // close output file

    fclose(xasm->ofile);
    return E_OK;

}

u32 fini_xasm(xasm* xasm){

    // finish xvm assembler

    close_ifile(xasm); xasm->ifile = NULL;
    close_ofile(xasm); xasm->ofile = NULL;
    fini_symtab(xasm->symtab); xasm->symtab = NULL;

    free(xasm); xasm = NULL;

    return E_OK;
}

char* strchrnul(const char *s, int c)
{
    char *ptr = strchr(s, c);
    if (!ptr) {
        ptr = strchr(s, '\0');
    }
    return ptr;
}

u32 get_sign(char s){

    if (s == '-'){
        return -1;
    }

    return 1;
}

u32 xasm_error(u32 error_id, u32 line, char* func){

    char* error_msg = NULL;

    switch (error_id) {
        case E_INVALID_IMMEDIATE: error_msg = "Invalid Immediate value";break;
        default: error_msg = "UNKNOWN ERROR";break;
    }
    printf("ERROR: %d:%s -- ", line, func);
    puts(error_msg);
    exit(error_id);
}

arg* init_arg(){
    // initialize argument

    arg* x_arg = (arg*) malloc(sizeof(arg));
    memset(x_arg, 0, sizeof(arg));
    return x_arg;
}

u32 disp_arg(arg* x_arg){
    printf("arg_type: ");
    switch(x_arg->arg_type){
        case arg_offset: printf("arg_offset\t");printf("offset: 0x%.8x\n", x_arg->opt_offset);break;
        case arg_register: printf("arg_register\t");printf("regid: 0x%x\n", x_arg->opt_regid);break;
        case arg_immediate: printf("arg_immediate\t");printf("value: 0x%.8x\n", x_arg->opt_value);break;
        case (arg_pointer | arg_offset| arg_register): printf("arg_pointer\t");printf("regid: 0x%x", x_arg->opt_regid);printf("offset: 0x%.8x\n", x_arg->opt_offset);break;
        case (arg_pointer | arg_offset): printf("arg_pointer\t");printf("offset: 0x%.8x\n", x_arg->opt_offset);break;
        case (arg_pointer | arg_register): printf("arg_pointer\t");printf("regid: 0x%x\n", x_arg->opt_regid);break;
        default: printf("arg_noarg\n");break;
    }
    return E_OK;
}

u32 fini_arg(arg* x_arg){
    // finish argument

    memset(x_arg, 0, sizeof(arg));
    free(x_arg); x_arg = NULL;
    return E_OK;
}