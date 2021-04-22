//
// Created by X3eRo0 on 2/23/2021.
//

#include "xasm.h"

const char* mnemonics[XVM_NINSTR] = {
        [XVM_OP_MOV]  = "mov",
        [XVM_OP_MOVB] = "movb",
        [XVM_OP_NOP]  = "nop",
        [XVM_OP_HLT]  = "hlt",
        [XVM_OP_RET]  = "ret",
        [XVM_OP_CALL] = "call",
        [XVM_OP_SYSC] = "syscall",
        [XVM_OP_ADD]  = "add",
        [XVM_OP_SUB]  = "sub",
        [XVM_OP_MUL]  = "mul",
        [XVM_OP_DIV]  = "div",
        [XVM_OP_XOR]  = "xor",
        [XVM_OP_AND]  = "and",
        [XVM_OP_OR]   = "or",
        [XVM_OP_NOT]  = "not",
        [XVM_OP_PUSH] = "push",
        [XVM_OP_POP]  = "pop",
        [XVM_OP_XCHG] = "xchg",
        [XVM_OP_INC]  = "inc",
        [XVM_OP_DEC]  = "dec",
        [XVM_OP_CMP]  = "cmp",
        [XVM_OP_JMP]  = "jmp",
        [XVM_OP_JZ]   = "jz",
        [XVM_OP_JNZ]  = "jnz",
        [XVM_OP_JA]   = "ja",
        [XVM_OP_JB]   = "jb",
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
            arg->arg_type = ARG_NARG;
        }
        return 0; // size of arguments are 0
    }

    // check if argument is register?

    if (args[0] == '$'){
        if (!calc_size) {
            arg->arg_type = ARG_REGD;
            arg->opt_regid = xasm_resolve_register_id(args);
        }
        return sizeof(u8); // size of register
    }

    // check if argument is immediate?

    if (args[0] == '#') {
        if (!calc_size) {
            arg->arg_type = ARG_IMMD;
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
                    arg->arg_type |= ARG_REGD;
                    arg->opt_regid = xasm_resolve_register_id(base);
                }
                size += sizeof(u8);
            }

            else if (base[0] == '#') { // offset
                if (!calc_size) {
                    arg->arg_type |= ARG_IMMD;
                    arg->opt_value = sign * xasm_resolve_number(base);
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
                    arg->arg_type |= ARG_IMMD;
                    arg->opt_value = sign * xasm_resolve_number(index);
                }
                size += sizeof(u32);
            }

            arg->arg_type |= ARG_PTRD;   // this class of argument is pointer
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
            arg->arg_type |= ARG_IMMD;
            arg->opt_value = i->addr;
            return E_OK;
        }
    }

    xasm_error(E_INVALID_SYNTAX, (u32) __LINE__ - 8, (char*)__PRETTY_FUNCTION__, "\"%s\" unrecognised argument", args);

    return E_ERR;
}

u32 xasm_assemble_line(xasm* xasm, char* line, section_entry** current_section_entry, bool calc_size){
    // assemble one line

    char opcds[128] = {0};
    char arg1s[128] = {0};
    char arg2s[128] = {0};

    char * temp = NULL;

    u32 size = 0; // size of instruction
    u32 opcd = 0;
    arg* arg1 = init_arg();
    arg* arg2 = init_arg();

    if (!strncmp(line, "section", 7)){ // defining section
        line += 7; // "section" skip these 7 bytes
        // skip to first non whitespace character

        clear_whitespaces(line);
        temp = line;
        skip_to_whitespace(line);
        *line++ = '\x00';
        *current_section_entry = find_section_entry_by_name(xasm->sections, temp);
        clear_whitespaces(line);

        char* flag_s = line;        // string for section flags
        skip_to_whitespace(flag_s)  // NULL terminate at first whitespace
        *flag_s++ = '\x00';         // after the immediate
        clear_whitespaces(flag_s);  // skip the whitespaces

        u32 section_flag = 0; // section flags
        u32 section_size = 0; // section size
        u32 section_addr = 0; // section addr

        section_addr = xasm_resolve_number(line);
        line = flag_s;
        skip_to_whitespace(flag_s);
        *flag_s++ = '\x00';
        section_size = xasm_resolve_number(line);
        line = flag_s;
        skip_to_whitespace(line);
        *line++ = '\x00';

        for (;flag_s[0] != '\x00' && flag_s[0] != '\x00';){
            if (flag_s[0] == '-' || is_white_space(flag_s)){
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

        // printf("section addr: 0x%x\n", section_addr);

        if (*current_section_entry != NULL) {
            (*current_section_entry)->v_addr = section_addr;
            (*current_section_entry)->v_size = section_size;
            (*current_section_entry)->m_flag = section_flag;
            (*current_section_entry)->m_buff = (char*)realloc((*current_section_entry)->m_buff, section_size);
        } else {
            *current_section_entry = add_section(xasm->sections, temp, section_size, section_addr, section_flag);
        }

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
                    write_buffer_to_section_by_addr(xasm->sections, (*current_section_entry)->v_addr, (u8)imm, WRITE_AS_BYTE);
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
                    write_buffer_to_section_by_name(xasm->sections, (*current_section_entry)->m_name, (u16)imm, WRITE_AS_WORD);
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
                    write_buffer_to_section_by_addr(xasm->sections, (*current_section_entry)->v_addr, (u32)imm, WRITE_AS_DWORD);
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

        // if token is a string
        if (line[0] == '"'){
            size = xasm_escape_string(line);
            if (!calc_size){
                memcpy_buffer_to_section_by_addr(xasm->sections, (*current_section_entry)->v_addr, line, size + 1); // +1 for null byte
            }
        } else {
            xasm_error(E_INVALID_SYNTAX, __LINE__, (char*)__PRETTY_FUNCTION__, "argument to 'ascii' does not begin with (\") : %s\n", line);
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
        size += 2; // size of opcode and mode byte in bytes

        size += xasm_resolve_argument(arg1, xasm->symtab, arg1s, calc_size); // get size of arguments
        size += xasm_resolve_argument(arg2, xasm->symtab, arg2s, calc_size); // get size of arguments

        if (!calc_size){ // instruction assembly
            // write opcode
            write_buffer_to_section_by_addr(xasm->sections, (*current_section_entry)->v_addr, (u8)opcd, WRITE_AS_BYTE);

            // write mode
            write_buffer_to_section_by_addr(xasm->sections, (*current_section_entry)->v_addr, arg1->arg_type | (arg2->arg_type << 0x4), WRITE_AS_BYTE);

            // write arg1
            if (arg1->arg_type != ARG_NARG){
                switch(arg1->arg_type){
                    case ARG_REGD: {
                        write_buffer_to_section_by_addr(xasm->sections, (*current_section_entry)->v_addr, arg1->opt_regid, WRITE_AS_BYTE); break;
                    }
                    case ARG_IMMD: {
                        write_buffer_to_section_by_addr(xasm->sections, (*current_section_entry)->v_addr, arg1->opt_value, WRITE_AS_DWORD); break;
                    }
                    default: {
                        if (arg1->arg_type & ARG_PTRD){
                            if (arg1->arg_type & ARG_REGD){
                                write_buffer_to_section_by_addr(xasm->sections, (*current_section_entry)->v_addr, arg1->opt_regid, WRITE_AS_BYTE);
                            }
                            if (arg1->arg_type & ARG_IMMD){
                                write_buffer_to_section_by_addr(xasm->sections, (*current_section_entry)->v_addr, arg1->opt_value, WRITE_AS_DWORD);
                            }
                        }
                    }
                }
            }
            // write arg2
            if (arg2->arg_type != ARG_NARG){
                switch(arg2->arg_type){
                    case ARG_REGD: {
                        write_buffer_to_section_by_addr(xasm->sections, (*current_section_entry)->v_addr, arg2->opt_regid, WRITE_AS_BYTE); break;
                    }
                    case ARG_IMMD: {
                        write_buffer_to_section_by_addr(xasm->sections, (*current_section_entry)->v_addr, arg2->opt_value, WRITE_AS_DWORD); break;
                    }
                    default: {
                        if (arg2->arg_type & ARG_PTRD){
                            if (arg2->arg_type & ARG_REGD){
                                write_buffer_to_section_by_addr(xasm->sections, (*current_section_entry)->v_addr, arg2->opt_regid, WRITE_AS_BYTE);
                            }
                            if (arg2->arg_type & ARG_IMMD){
                                write_buffer_to_section_by_addr(xasm->sections, (*current_section_entry)->v_addr, arg2->opt_value, WRITE_AS_DWORD);
                            }
                        }
                    }
                }
            }
        }

    } else {
        fini_arg(arg1); arg1 = NULL;
        fini_arg(arg2); arg2 = NULL;
        xasm_error(E_INVALID_SYNTAX, (u32) __LINE__ - 16, (char*) __PRETTY_FUNCTION__, " Invalid Instruction \"%s\"", line);
    }

    fini_arg(arg1); arg1 = NULL;
    fini_arg(arg2); arg2 = NULL;

    return size;
}


u32 xasm_assemble(xasm* xasm, section_entry* default_section_entry){
    // assemble the xvm source

    char* line      = NULL; // this buffer will be allocated by getline
    char* temp      = NULL;
    char* comment   = NULL; // pointer to comment
    char* label     = NULL; // pointer to label
    char* newline   = NULL; // pointer to '\n'
    size_t size     = 0;    // size of line

    section_entry* current_section = default_section_entry;

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

        clear_whitespaces(temp);

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
            add_symbol(xasm->symtab, temp, current_section->a_size + current_section->v_addr); // append the symbol
            temp = ++label;         // process the rest of the string
            if (*temp == '\x00') {  // if the string ends here continue
                continue;
            }
        }

        // instruction is valid
        // increment address
        current_section->a_size += xasm_assemble_line(xasm, temp, &current_section, true); // get size
    }

    rewind(xasm->ifile);    // rewind the file
    reset_address_of_sections(xasm->sections);
    current_section = default_section_entry;

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
        xasm_assemble_line(xasm, temp, &current_section, false); // assemble
    }


    free(line);
    line = NULL; temp = NULL; size = 0;

    return E_OK;
}