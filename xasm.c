//
// Created by X3eRo0 on 2/23/2021.
//

#include "xasm.h"

const char* mnemonics[XVM_NINSTR] = {
        [opc_mov] = "mov",
};

const char* registers[XVM_NREGS] = {
        [reg_r0] = "$r0",
        [reg_r1] = "$r1",
        [reg_r2] = "$r2",
        [reg_r3] = "$r3",
        [reg_r4] = "$r4",
        [reg_pc] = "$pc",
        [reg_sp] = "$sp",
};



u32 xasm_resolve_number(char* num_s){
    // detect base and return number

    if (*num_s == '#'){
        // just in case if we get an
        // input starting with '#'
        num_s++;
    }

    char* z_ptr = strchr(num_s, '0');
    char* x_ptr = strchr(num_s, 'x');
    char* b_ptr = strchr(num_s, 'b');
    char* o_ptr = strchr(num_s, 'o');
    char* temp  = NULL;

    // hexadecimal numbers
    if (z_ptr && x_ptr) {

        temp = ++x_ptr; // points to digits
        for (; *temp != '\x00'; temp++){
            if(!is_digit(*temp)){
                printf("%c\n", *temp);
                xasm_error(E_INVALID_IMMEDIATE, (u32) __LINE__ - 1, (char*) __PRETTY_FUNCTION__);
            }
        }

        return (u32) strtol(x_ptr, NULL, 16);
    }

    // octal numbers
    if (z_ptr && o_ptr) {

        temp = ++o_ptr; //points to digits
        for (; *temp != '\x00'; temp++) {
            if (!is_digit(*temp)) {
                xasm_error(E_INVALID_IMMEDIATE, (u32) __LINE__ - 1, (char *) __PRETTY_FUNCTION__);
            }
        }
        return (u32) strtol(o_ptr, NULL, 8);
    }

    // binary numbers
    if (z_ptr && b_ptr) {

        temp = ++b_ptr; //points to digits
        for (; *temp != '\x00'; temp++){
            if (!is_binary(*temp)){
                xasm_error(E_INVALID_IMMEDIATE, (u32) __LINE__ - 1, (char*) __PRETTY_FUNCTION__);
            }
        }
        return (u32) strtol(b_ptr, NULL, 2);
    }


    // decimal numbers
    if (!z_ptr && !x_ptr && !b_ptr && !o_ptr) {
        return (u32) strtol(num_s, NULL, 10);
    } else {
        // invalid base error out
        xasm_error(E_INVALID_IMMEDIATE, (u32) __LINE__ - 1, (char*) __PRETTY_FUNCTION__);
    }

    return E_ERR;
}

char* xasm_resolve_mnemonic(u32 opcode){
    // resolve opcode's mnemonic

    return opcode < XVM_NINSTR ? (char*)mnemonics[opcode] : NULL;
}

u32 xasm_resolve_opcode(char* args){
    // resolve mnemonic's opcode

    for (u32 i = 0; i < XVM_NINSTR; i++){
        if (!strncmp(xasm_resolve_mnemonic(i), args, strlen(args))){
            return i;
        }
    }

    return E_ERR;
}

char* xasm_resolve_register(u32 reg_id){
    // return register string

    return reg_id < XVM_NREGS ? (char*)registers[reg_id] : NULL;
}

u32 xasm_resolve_register_id(char* reg_s){
    // resolve register id

    for (u32 i = 0; i < XVM_NREGS; i++){
        if (!strncmp(xasm_resolve_register(i), reg_s, strlen(reg_s))){
            return i;
        }
    }

    return E_ERR;
}

u32 xasm_resolve_argument(arg* arg, symtab* symtab, char* args){
    // check if argument exists

    if (args[0] == '\x00'){
        arg->arg_type = arg_noarg;
        return E_OK;
    }

    // check if argument is register?

    if (args[0] == '$'){
        arg->arg_type = arg_register;
        arg->opt_regid = xasm_resolve_register_id(args);
        return E_OK;
    }

    // check if argument is immediate?

    if (args[0] == '#'){
        arg->arg_type = arg_immediate;
        arg->opt_value = xasm_resolve_number(++args);
        return E_OK;
    }

    // check if argument if memory location with offset

    if (args[0] == '['){
        char index[128] = {0};
        char base[128] = {0};
        char modifier[128] = {0};
        u32  sign = 1; // +ve

        arg->arg_type = arg_pointer;    // this class of argument is pointer
                                        // with or without (offset & register)

        // this is magic XD
        if (sscanf(args, "[%127[^] +-] %1[+-] %127[^]]]", base, modifier, index) >= 1) {

            // check if base exists
            if (base[0] == '\x00'){ // bail out if it's invalid code
                return E_ERR;
            }

            // check if base is register or immediate

            if (base[0] == '$'){ // register
                arg->arg_type |= arg_register;
                arg->opt_regid = xasm_resolve_register_id(base);
            }

            if (base[0] == '#') { // offset
                arg->arg_type |= arg_offset;
                arg->opt_offset = sign * xasm_resolve_number(base);
            }

            if (base[0] == '+' || base[0] == '-'){
                sign = get_sign(base[0]);
            }

            if (modifier[0] != '\x00' && (modifier[0] == '+' || modifier[0] == '-')){
                sign = get_sign(modifier[0]);
            }

            if (index[0] != '\x00' && index[0] == '#'){
                arg->arg_type |= arg_offset;
                arg->opt_offset = sign * xasm_resolve_number(index);
            }

        }

        return E_OK;

    }

    // check if argument is label
    for (sym_entry * i = symtab->symbols; i != NULL; i = i->next){
        if (!strncmp(i->name, args, strlen(args))){
            arg->arg_type |= arg_offset;
            arg->opt_offset = i->addr;
            return E_OK;
        }
    }


    return E_ERR;
}

u32 xasm_assemble(xasm* xasm){
    // assemble the xvm source

    char* line      = NULL; // this buffer will be allocated by getline
    char* temp      = NULL;
    char* comment   = NULL; // pointer to comment
    char* label     = NULL; // pointer to label
    char* newline   = NULL; // pointer to '\n'
    u32 address     = 0;    // for recording addresses
    size_t size     = 0;    // size of line

    if (xasm == NULL) {
        return E_ERR;
    }

    if (xasm->ifile == NULL) {
        return E_ERR;
    }

    if (xasm->ofile == NULL) {
        return E_ERR;
    }

    // first pass

    while (getline(&line, &size, xasm->ifile) > 0){
        temp = line;

        while (is_white_space(temp)){
            temp++;
        }

        comment = strchrnul(temp, ';');     // find comment
        label   = strchrnul(temp, ':');     // find label
        newline = strchrnul(temp, '\n');    // find line end

        // remove any comment or label symbol
        comment[0]  = '\x00';
        label[0]    = '\x00';
        newline[0]  = '\x00';

        if (is_line_empty(temp)){
            continue;
        }

        if (label < comment) {
            add_symbol(xasm->symtab, temp, address); // append the symbol
            temp = ++label;         // process the rest of the string
            if (*temp == '\x00') {  // if the string ends here continue
                continue;
            }
        }

        // instruction is valid
        // increment address
        address += xasm_assemble_line(NULL, temp); // get size
    }

    free(line);
    line = NULL; temp = NULL; size = 0;

    display_symtab(xasm->symtab);
    return E_OK;
}

u32 xasm_assemble_line(xasm* xasm, char* line){
    // assemble one line

    char opcds[128] = {0};
    char arg1s[128] = {0};
    char arg2s[128] = {0};

    u32 opcd = 0;
    arg* arg1 = init_arg();
    arg* arg2 = init_arg();

    if (sscanf(line, "%127s %127[^,\t\n], %127[^\t\n]", opcds, arg1s, arg2s) >= 1){
        // to check if instruction was
        // correct for pass 1
        // return size if xasm == NULL

        if (xasm == NULL) {
            // calculate size
        }

        opcd = xasm_resolve_opcode(opcds);
        if (opcd == E_ERR ||
            xasm_resolve_argument(arg1, xasm->symtab, arg1s) == E_ERR ||
            xasm_resolve_argument(arg2, xasm->symtab, arg2s) == E_ERR
        ) {
            fini_arg(arg1); arg1 = NULL;
            fini_arg(arg2); arg2 = NULL;
            return E_ERR;
        }

        printf("arg1s: \"%s\"\t", arg1s); disp_arg(arg1);
        printf("arg2s: \"%s\"\t", arg2s); disp_arg(arg2);

    } else {
        fini_arg(arg1); arg1 = NULL;
        fini_arg(arg2); arg2 = NULL;
        return E_ERR;
    }

    fini_arg(arg1); arg1 = NULL;
    fini_arg(arg2); arg2 = NULL;

    return E_OK;
}


int main(int argc, char* argv[]){


    xasm* xasm = init_xasm();

    if (argc != 3) {
        printf("Usage %s <input.asm> <output.bin>\n", argv[0]);
        return E_ERR;
    }

    open_ifile(xasm, argv[1]);
    open_ofile(xasm, argv[2]);
    xasm_assemble(xasm);
    fini_xasm(xasm);
    xasm = NULL;
    return E_OK;

}