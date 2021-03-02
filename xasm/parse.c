//
// Created by X3eRo0 on 2/23/2021.
//

#include "xasm.h"

const char* mnemonics[XVM_NINSTR] = {
        [opc_mov] = "mov",
        [opc_hlt] = "hlt",
        [opc_ret] = "ret",
        [opc_call] = "call",
        [opc_syscall] = "syscall",
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

    xasm_error(E_INVALID_OPCODE, (u32)__LINE__ - 5, (char*)__PRETTY_FUNCTION__, "Unknown Opcode : \"%s\"", args);
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

u32 xasm_resolve_argument(arg* arg, symtab* symtab, char* args, bool calc_size){
    // check if argument exists

    if (args[0] == '\x00'){
        if (!calc_size){
            arg->arg_type = arg_noarg;
        }
        return 0; // size of arguments are 0
    }

    // check if argument is register?

    if (args[0] == '$'){
        if (!calc_size) {
            arg->arg_type = arg_register;
            arg->opt_regid = xasm_resolve_register_id(args);
        }
        return sizeof(u8); // size of register
    }

    // check if argument is immediate?

    if (args[0] == '#') {
        if (!calc_size) {
            arg->arg_type = arg_immediate;
            arg->opt_value = xasm_resolve_number(++args);
        }
        return sizeof(u32); // size of immediate
    }

    // check if argument if memory location with offset

    if (args[0] == '['){
        char index[128] = {0};
        char base[128] = {0};
        char modifier[128] = {0};
        u32  sign = 1; // +ve
        u32  size = 0;


        // this is magic XD
        if (sscanf(args, "[%127[^] +-] %1[+-] %127[^]]]", base, modifier, index) >= 1) {

            // check if base is register or immediate

            if (base[0] == '$'){ // register
                if (!calc_size) {
                    arg->arg_type |= arg_register;
                    arg->opt_regid = xasm_resolve_register_id(base);
                }
                size += sizeof(u8);
            }

            else if (base[0] == '#') { // offset
                if (!calc_size) {
                    arg->arg_type |= arg_offset;
                    arg->opt_offset = sign * xasm_resolve_number(base);
                }
                size += sizeof(u32);
            }

            else if (base[0] == '+' || base[0] == '-'){
                sign = get_sign(base[0]);
            }

            else {
                xasm_error(E_INVALID_SYNTAX, (u32)__LINE__, (char*)__PRETTY_FUNCTION__, "invalid argument : \"%s\"", args);
            }

            if (modifier[0] != '\x00' && (modifier[0] == '+' || modifier[0] == '-')){
                sign = get_sign(modifier[0]);
            }

            if (index[0] != '\x00' && index[0] == '#'){
                if (!calc_size) {
                    arg->arg_type |= arg_offset;
                    arg->opt_offset = sign * xasm_resolve_number(index);
                }
                size += sizeof(u32);
            }

            arg->arg_type |= arg_pointer;   // this class of argument is pointer
                                            // with or without (offset & register)
        } else {
            xasm_error(E_INVALID_SYNTAX, (u32)__LINE__ - 1, (char*)__PRETTY_FUNCTION__, "invalid argument : \"%s\"", args);
        }

        return size;

    }

    // in case the argument is not either of the above
    // assume it's a label and return address size
    if (calc_size){
        return sizeof(u32);
    }

    // check if argument is label
    for (sym_entry * i = symtab->symbols; i != NULL; i = i->next){
        if (!strncmp(i->name, args, strlen(args))){
            arg->arg_type |= arg_offset;
            arg->opt_offset = i->addr;
            return E_OK;
        }
    }

    xasm_error(E_INVALID_SYNTAX, (u32) __LINE__ - 8, (char*)__PRETTY_FUNCTION__, "\"%s\" unrecognised argument", args);

    return E_ERR;
}

u32 xasm_assemble_line(xasm* xasm, char* line, bool calc_size){
    // assemble one line

    char opcds[128] = {0};
    char arg1s[128] = {0};
    char arg2s[128] = {0};

    char *sec_name = section_name;

    u32 size = 0; // size of instruction
    u32 opcd = 0;
    arg* arg1 = init_arg();
    arg* arg2 = init_arg();

    if (!strncmp(line, "section", 7)){ // defining section
        line += 7; // "section" skip these 7 bytes
        // skip to first non whitespace character

        clear_whitespaces(line);
        sec_name = line;
        skip_to_whitespace(line);
        *line++ = '\x00';
        clear_whitespaces(line);

        char* flag_s = line;        // string for section flags
        skip_to_whitespace(flag_s)  // NULL terminate at first whitespace
        *flag_s++ = '\x00';         // after the immediate
        clear_whitespaces(flag_s);  // skip the whitespaces

        u32 section_flag = 0; // section flags
        u32 section_size = 0; // section size

        section_size = xasm_resolve_number(line);

        for (;flag_s[0] != '\x00' && flag_s[0] != '\x00';){
            if (flag_s[0] == '|' || is_white_space(flag_s)){
                flag_s++;
            }

            if ((flag_s[0] | ' ') == 'r'){
                section_flag |= PERM_READ;
                flag_s++;
            }

            if ((flag_s[0] | ' ') == 'w'){
                section_flag |= PERM_WRITE;
                flag_s++;
            }

            if ((flag_s[0] | ' ') == 'x'){
                section_flag |= PERM_EXEC;
                flag_s++;
            }
        }

        section_name = add_section(xasm->sections, sec_name, section_size, section_flag);
        fini_arg(arg1); arg1 = NULL;
        fini_arg(arg2); arg2 = NULL;
        return 0;
    }

    // check if line is a valid db, dw, dd, ascii command

    if (!strncmp(line, "db", 2)){ // db
        line += 2; // "db" skip these 2 bytes
        // skip to first non whitespace character

        clear_whitespaces(line);

        char* rest  = line;
        char* token = NULL;
        while ((token = strtok_r(rest, ",", &rest))){
            clear_whitespaces(token);
            // if token is an u8 immediate
            if (token[0] == '#'){
                size += sizeof(u8);

                if (!calc_size){
                    u32 imm = xasm_resolve_number(token);
                    write_buffer_to_section_by_name(xasm->sections, section_name, (u8)imm, WRITE_AS_BYTE);
                }
            }
        }

        fini_arg(arg1); arg1 = NULL;
        fini_arg(arg2); arg2 = NULL;
        return size;
    }

    // check if line is a valid db or ascii

    if (!strncmp(line, "dw", 2)){ // dw
        line += 2; // "dw" skip these 2 bytes
        // skip to first non whitespace character

        clear_whitespaces(line);

        char* rest  = line;
        char* token = NULL;
        while ((token = strtok_r(rest, ",", &rest))){
            clear_whitespaces(token);
            // if token is an u16 immediate
            if (token[0] == '#'){
                size += sizeof(u16);

                if (!calc_size){
                    u32 imm = xasm_resolve_number(token);
                    write_buffer_to_section_by_name(xasm->sections, section_name, (u16)imm, WRITE_AS_WORD);
                }
            }
        }

        fini_arg(arg1); arg1 = NULL;
        fini_arg(arg2); arg2 = NULL;
        return size;
    }

    if (!strncmp(line, "dd", 2)){ // dd
        line += 2; // "dd" skip these 2 bytes
        // skip to first non whitespace character

        clear_whitespaces(line);

        char* rest  = line;
        char* token = NULL;
        while ((token = strtok_r(rest, ",", &rest))){
            clear_whitespaces(token);
            // if token is an u32 immediate
            if (token[0] == '#'){
                size += sizeof(u32);

                if (!calc_size){
                    u32 imm = xasm_resolve_number(token);
                    write_buffer_to_section_by_name(xasm->sections, section_name, (u32)imm, WRITE_AS_DWORD);
                }
            }
        }

        fini_arg(arg1); arg1 = NULL;
        fini_arg(arg2); arg2 = NULL;
        return size;
    }

    if (!strncmp(line, "ascii", 5)){ // ascii
        line += 5; // "ascii" skip these 5 bytes
        // skip to first non whitespace character

        clear_whitespaces(line);

        char* rest  = line;
        char* token = NULL;
        while ((token = strtok_r(rest, ",", &rest))){
            clear_whitespaces(token);

            // if token is a string
            if (token[0] == '"'){
                size = xasm_escape_string(token);
                if (!calc_size){
                    memcpy_buffer_to_section_by_name(xasm->sections, section_name, token, size + 1); // +1 for null byte
                }
            } else {
                xasm_error(E_INVALID_SYNTAX, __LINE__, (char*)__PRETTY_FUNCTION__, "argument to 'ascii' does not begin with (\") : %s\n", token);
            }
        }

        fini_arg(arg1); arg1 = NULL;
        fini_arg(arg2); arg2 = NULL;
        return size;
    }


    // check if line is a valid instruction
    if (sscanf(line, "%127s %127[^,\t\n], %127[^\t\n]", opcds, arg1s, arg2s) >= 1){
        // to check if instruction was
        // correct for pass 1

        opcd = xasm_resolve_opcode(opcds);
        size += 1; // size of opcode in bytes

        size += xasm_resolve_argument(arg1, xasm->symtab, arg1s, calc_size); // get size of arguments
        size += xasm_resolve_argument(arg2, xasm->symtab, arg2s, calc_size); // get size of arguments

        if (!calc_size){
            write_buffer_to_section_by_name(xasm->sections, section_name, (u8)opcd, WRITE_AS_BYTE);
        }

    } else {
        fini_arg(arg1); arg1 = NULL;
        fini_arg(arg2); arg2 = NULL;
        xasm_error(E_INVALID_SYNTAX, (u32) __LINE__ - 16, (char*) __PRETTY_FUNCTION__, "Invalid Instruction \"%s\"", line);
    }

    fini_arg(arg1); arg1 = NULL;
    fini_arg(arg2); arg2 = NULL;

    return size;
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

    section_name = ".text"; // default section

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
        address += xasm_assemble_line(xasm, temp, true); // get size
    }

    rewind(xasm->ifile);    // rewind the file
    address = 0;            // reset address

    // second pass
    while(getline(&line, &size, xasm->ifile) > 0){
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
            continue;
        }

        // instruction is valid
        // increment address
        xasm_assemble_line(xasm, temp, false); // assemble
    }


    free(line);
    line = NULL; temp = NULL; size = 0;

    display_symtab(xasm->symtab);
    return E_OK;
}