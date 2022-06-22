//
// Created by X3eRo0 on 2/23/2021.
//

#include <xasm.h>


char* xasm_resolve_mnemonic(u32 opcode){
    // resolve opcode's mnemonic

    return opcode < (sizeof(mnemonics)/sizeof(char *)) ? (char*)mnemonics[opcode] : NULL;
}

u32 xasm_resolve_opcode(char* args){
    // resolve mnemonic's opcode

    char * mnemonic = NULL;

    for (u32 i = 0; i < sizeof(mnemonics)/sizeof(char *); i++){
        mnemonic = xasm_resolve_mnemonic(i);
        if (mnemonic == NULL){
            continue;
        }

        if (!strncmp(mnemonic, args, strlen(args))){
            return i;
        }
    }

    xasm_error(E_INVALID_OPCODE, (u32)__LINE__ - 5, (char*)__PRETTY_FUNCTION__, "Unknown Opcode : \"%s\"", args);
    return E_ERR;
}

char* xasm_resolve_register(u32 reg_id){
    // return register string

    return reg_id < XVM_NREGS ? (char*)regid_2_str[reg_id] : NULL;
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

u32 xasm_resolve_argument(arg* arg, xasm* xasm, char* args, bool calc_size){
    // check if argument exists

    if (args[0] == '\x00'){
        if (!calc_size){
            arg->arg_type = ARG_NARG;
        }
        return 0; // size of arguments are 0
    }

    char * end = strchr(args, '\x00') - 1;
    if (is_white_space(end)){
        while (is_white_space(end) && args > end){
            end--;
        }

        *++end = '\0';
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

                if (calc_size){
                    return sizeof(u32);
                }

                // check if base is a symbol or define

                for (sym_entry * i = xasm->symtab->symbols; i != NULL; i = i->next){
                    if (!strncmp(i->name, base, strlen(base))){
                        arg->arg_type |= ARG_IMMD | ARG_PTRD;
                        arg->opt_value = i->addr;
                        return E_OK;
                    }
                }

                for (sym_entry * i = xasm->define->symbols; i != NULL; i = i->next){
                    if (!strncmp(i->name, base, strlen(base))){
                        arg->arg_type |= ARG_IMMD | ARG_PTRD;
                        arg->opt_value = i->addr;
                        return E_OK;
                    }
                }

                xasm_error(E_INVALID_SYNTAX, (u32)__LINE__, (char*)__PRETTY_FUNCTION__, "invalid argument : \"%s\"", base);
            }

            if (modifier[0] != '\x00' && (modifier[0] == '+' || modifier[0] == '-')){
                sign = get_sign(modifier[0]);

                if (index[0] != '\x00') {
                    if (!calc_size) {
                        arg->arg_type |= ARG_IMMD;
                        arg->opt_value = sign * xasm_resolve_number(index);
                    }
                    size += sizeof(u32);
                }
            }

            arg->arg_type |= ARG_PTRD;   // this class of argument is pointer
                                         // with or without (offset & register)
        } else {
            xasm_error(E_INVALID_SYNTAX, (u32)__LINE__ - 1, (char*)__PRETTY_FUNCTION__, "invalid argument : \"%s\"", args);
        }

        return size;

    }

    // in case the argument is not either of the above
    // assume it's a label or define and return address size
    if (calc_size){
        return sizeof(u32);
    }

    // check if argument is label
    for (sym_entry * i = xasm->symtab->symbols; i != NULL; i = i->next){
        if (!strncmp(i->name, args, strlen(args))){
            arg->arg_type |= ARG_IMMD;
            arg->opt_value = i->addr;
            return E_OK;
        }
    }

    // check if argument is define
    for (sym_entry * i = xasm->define->symbols; i != NULL; i = i->next){
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

    if (!strncmp(line, ".section", 7)){ // defining section
        line += 8; // "section" skip these 7 bytes
        // skip to first non whitespace character

        clear_whitespaces(line);
        temp = line;
        skip_to_whitespace(line);
        *line++ = '\x00';
        *current_section_entry = NULL;

        // find section
        section_entry* tent = xasm->sections->sections;

        while (tent != NULL){
            if (tent->m_name != NULL && !strncmp(tent->m_name, temp, strlen(tent->m_name))){
                *current_section_entry = tent;
            }
            tent = tent->next;
        }

        clear_whitespaces(line);

        if (*line == '\x00'){
            fini_arg(arg1); arg1 = NULL;
            fini_arg(arg2); arg2 = NULL;
            return 0;
        }

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

    // check if line is a valid db, dw, dd, .asciz command

    if (!strncmp(line, ".db", 3)){ // db
        line += 3; // "db" skip these 2 bytes
        // skip to first non whitespace character

        clear_whitespaces(line);

        char* rest  = line;
        char* token = NULL;
        while ((token = strtok_r(rest, ",", &rest))){
            clear_whitespaces(token);
            size += sizeof(u8);
            // if token is an u8 immediate
            if (token[0] == '#'){

                if (!calc_size){
                    u32 imm = xasm_resolve_number(token);
                    write_buffer_to_section_by_addr(xasm->sections, (*current_section_entry)->v_addr, (u32)imm, WRITE_AS_BYTE);
                }
            } else if (!calc_size) {
                u32 imm = resolve_symbol_addr(xasm->symtab, token);
                if (imm == E_ERR){
                    xasm_error(E_INVALID_SYNTAX, 0, NULL, "\"%s\" Not Defined", token);
                }
                write_buffer_to_section_by_addr(xasm->sections, (*current_section_entry)->v_addr, (u32)imm, WRITE_AS_BYTE);
            }
        }

        fini_arg(arg1); arg1 = NULL;
        fini_arg(arg2); arg2 = NULL;
        return size;
    }

    // check if line is a valid db or asciz

    if (!strncmp(line, ".dw", 3)){ // dw
        line += 3; // "dw" skip these 2 bytes
        // skip to first non whitespace character

        clear_whitespaces(line);

        char* rest  = line;
        char* token = NULL;
        while ((token = strtok_r(rest, ",", &rest))){
            clear_whitespaces(token);
            size += sizeof(u16);
            // if token is an u16 immediate
            if (token[0] == '#'){

                if (!calc_size){
                    u32 imm = xasm_resolve_number(token);
                    write_buffer_to_section_by_addr(xasm->sections, (*current_section_entry)->v_addr, (u32)imm, WRITE_AS_WORD);
                }
            } else if (!calc_size) {
                u32 imm = resolve_symbol_addr(xasm->symtab, token);
                if (imm == E_ERR){
                    xasm_error(E_INVALID_SYNTAX, 0, NULL, "\"%s\" Not Defined", token);
                }
                write_buffer_to_section_by_addr(xasm->sections, (*current_section_entry)->v_addr, (u32)imm, WRITE_AS_WORD);
            }
        }

        fini_arg(arg1); arg1 = NULL;
        fini_arg(arg2); arg2 = NULL;
        return size;
    }

    if (!strncmp(line, ".dd", 3)){ // dd
        line += 3; // "dd" skip these 2 bytes
        // skip to first non whitespace character

        clear_whitespaces(line);

        char* rest  = line;
        char* token = NULL;
        while ((token = strtok_r(rest, ",", &rest))){
            clear_whitespaces(token);
            size += sizeof(u32);
            // if token is an u32 immediate
            if (token[0] == '#'){

                if (!calc_size){
                    u32 imm = xasm_resolve_number(token);
                    write_buffer_to_section_by_addr(xasm->sections, (*current_section_entry)->v_addr, (u32)imm, WRITE_AS_DWORD);
                }
            } else if (!calc_size) {
                u32 imm = resolve_symbol_addr(xasm->symtab, token);
                if (imm == E_ERR){
                    xasm_error(E_INVALID_SYNTAX, 0, NULL, "\"%s\" Not Defined", token);
                }
                write_buffer_to_section_by_addr(xasm->sections, (*current_section_entry)->v_addr, (u32)imm, WRITE_AS_DWORD);
            }
        }

        fini_arg(arg1); arg1 = NULL;
        fini_arg(arg2); arg2 = NULL;
        return size;
    }

    if (!strncmp(line, ".asciz", 6)){ // asciz
        line += 6; // "asciz" skip these 5 bytes
        // skip to first non whitespace character

        clear_whitespaces(line);

        // if token is a string
        if (line[0] == '"'){
            size = xasm_escape_string(line) + 1;
            if (!calc_size){
                memcpy_buffer_to_section_by_addr(xasm->sections, (*current_section_entry)->v_addr, line, size); // +1 for null byte
            }
        } else {
            xasm_error(E_INVALID_SYNTAX, __LINE__, (char*)__PRETTY_FUNCTION__, "argument to '.asciz' does not begin with (\") : %s\n", line);
        }

        fini_arg(arg1); arg1 = NULL;
        fini_arg(arg2); arg2 = NULL;
        return size;
    }

    if (!strncmp(line, "#define", 7)){
        fini_arg(arg1); arg1 = NULL;
        fini_arg(arg2); arg2 = NULL;
        return 0;
    }

    // check if line is a valid instruction
    if (sscanf(line, "%127s %127[^,\t\n], %127[^\t\n]", opcds, arg1s, arg2s) >= 1){
        // to check if instruction was
        // correct for pass 1

        opcd = xasm_resolve_opcode(opcds);
        size += 2; // size of opcode and mode byte in bytes

        size += xasm_resolve_argument(arg1, xasm, arg1s, calc_size); // get size of arguments
        size += xasm_resolve_argument(arg2, xasm, arg2s, calc_size); // get size of arguments

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


u32 xasm_assemble(xasm *xasm, section_entry *default_section_entry, FILE **inputf, u32 ifiles) {
    // assemble the xvm source

    char* line      = NULL; // this buffer will be allocated by getline
    char* temp      = NULL;
    char* comment   = NULL; // pointer to comment
    char* label     = NULL; // pointer to label
    char* newline   = NULL; // pointer to '\n'
    char* define    = NULL; // pointer to '#'
    char* asciz     = NULL; // pointer to ".asciz"
    size_t size     = 0;    // size of line

    section_entry* current_section = default_section_entry;

    if (xasm == NULL) {
        return E_ERR;
    }

    if (xasm->ofile == NULL) {
        return E_ERR;
    }

    // first pass for all input files

    for (u32 i = 0; i < ifiles; i++) {

        xasm->ifile = inputf[i];

        while (getline(&line, &size, xasm->ifile) > 0) {
            temp = line;

            clear_whitespaces(temp);

            asciz = strstr(temp, ".asciz");         // find asciz
            if (asciz == NULL) {
                comment = strchrnul(temp, ';');         // find comment
                label = strchrnul(temp, ':');           // find label
                define = strstr(temp, "#define");   // find defines
                newline = strchrnul(temp, '\n');        // find line end

                // remove any comment or label symbol
                comment[0] = '\x00';
                label[0] = '\x00';
                newline[0] = '\x00';

            }


            if (is_line_empty(temp)) {
                continue;
            }

            if (define != NULL && !strncmp(define, "#define", 7)){
                define += 7;
                clear_whitespaces(define);
                temp = define;
                skip_to_whitespace(define);
                clear_whitespaces(define);
                add_symbol(xasm->define, temp, xasm_resolve_number(define));
                continue;
            }

            if (label < comment && asciz == NULL) {
                add_symbol(xasm->symtab, temp, current_section->a_size + current_section->v_addr); // append the symbol
                temp = ++label;         // process the rest of the string
                clear_whitespaces(temp);
                if (*temp == '\x00') {  // if the string ends here continue
                    continue;
                }
            }

            // instruction is valid
            // increment address
            current_section->a_size += xasm_assemble_line(xasm, temp, &current_section, true); // get size
        }

        rewind(xasm->ifile);    // rewind the file
        current_section = default_section_entry;
    }

    reset_address_of_sections(xasm->sections);

    // second pass for all the input files

    for (u32 i = 0; i < ifiles; i++) {

        xasm->ifile = inputf[i];
        current_section = default_section_entry;

        while (getline(&line, &size, xasm->ifile) > 0) {
            temp = line;

            clear_whitespaces(temp);

            asciz = strstr(temp, ".asciz");
            if (asciz == NULL) {
                comment = strchrnul(temp, ';');     // find comment
                label = strchrnul(temp, ':');     // find label
                newline = strchrnul(temp, '\n');    // find line end

                // remove any comment or label symbol
                comment[0] = '\x00';
                label[0] = '\x00';
                newline[0] = '\x00';
            }

            if (is_line_empty(temp)) {
                continue;
            }

            if (label < comment && asciz == NULL) {
                continue;
            }

            // instruction is valid
            // increment address
            xasm_assemble_line(xasm, temp, &current_section, false); // assemble
        }

        free(line);
        line = NULL; temp = NULL; size = 0;
    }


    return E_OK;
}
