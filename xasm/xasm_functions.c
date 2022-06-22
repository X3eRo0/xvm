//
// Created by X3eRo0 on 2/23/2021.
//

#include <xasm.h>

xasm* init_xasm()
{

    // initialize xvm assembler

    xasm* t_xasm = (xasm*)malloc(sizeof(xasm));
    t_xasm->ifile = NULL;
    t_xasm->ofile = NULL;
    t_xasm->symtab = init_symtab();
    t_xasm->define = init_symtab();
    t_xasm->header = init_exe_header();

    return t_xasm;
}

u32 xasm_open_ifile(xasm* xasm, char* file)
{

    // open input file

    xasm->ifile = fopen(file, "rb");
    if (!xasm->ifile) {
        xasm_error(E_NOFILE, (u32)__LINE__, (char*)__PRETTY_FUNCTION__, "No such file \"%s\" found", file);
    }
    return E_OK;
}

u32 xasm_open_ofile(xasm* xasm, char* file)
{

    // open output file

    xasm->ofile = fopen(file, "wb");
    if (!xasm->ofile) {
        xasm_error(E_NOFILE, (u32)__LINE__, (char*)__PRETTY_FUNCTION__, "Cannot create \"%s\"", file);
    }
    return E_OK;
}

u32 xasm_close_ifile(xasm* xasm)
{

    // close input file

    if (xasm->ifile) {
        fclose(xasm->ifile);
    }
    return E_OK;
}

u32 xasm_close_ofile(xasm* xasm)
{

    // close output file

    if (xasm->ofile) {
        fclose(xasm->ofile);
    }
    return E_OK;
}

u32 get_total_size(xasm* xasm)
{

    section_entry* sec_temp = xasm->sections->sections;
    sym_entry* sym_temp = xasm->symtab->symbols;

    u32 total = XVM_HDR_SIZE + xasm->symtab->n_symbols * sizeof(u32) * 2;

    while (sym_temp != NULL) {
        total += strlen(sym_temp->name) + 1;
        sym_temp = sym_temp->next;
    }

    while (sec_temp != NULL) {
        total += strlen(sec_temp->m_name) + 1;
        total += sizeof(u32) * 3; // for size, flag and indx
        total += sec_temp->a_size; // content of that section
        sec_temp = sec_temp->next;
    }

    return total;
}

u32 fini_xasm(xasm* xasm)
{

    // finish xvm assembler

    xasm_close_ofile(xasm);
    xasm->ofile = NULL;
    fini_symtab(xasm->symtab);
    xasm->symtab = NULL;
    fini_symtab(xasm->define);
    xasm->define = NULL;
    fini_section(xasm->sections);
    xasm->sections = NULL;
    fini_exe_header(xasm->header);
    xasm->header = NULL;

    free(xasm);
    xasm = NULL;

    return E_OK;
}

char* strchrnul(const char* s, int c)
{
    char* ptr = strchr(s, c);
    if (!ptr) {
        ptr = strchr(s, '\0');
    }
    return ptr;
}

u32 get_sign(char s)
{

    if (s == '-') {
        return -1;
    }

    return 1;
}

#pragma clang diagnostic push
#pragma ide diagnostic ignored "cert-err34-c"
u32 xasm_escape_string(char* unescaped)
{
    char* writeptr = unescaped++;
    u32 size = 0;
    while (unescaped[0] != '"') {
        if (unescaped[0] == '\\') {
            // if an escape sequence is found
            // process it and update the current string
            // escaped string will always be equal to or
            // smaller than the unescaped string, so we
            // can use the same memory for escaping.
            switch (unescaped[1]) {
            case '0':
                writeptr[0] = '\0';
                break;
            case 'n':
                writeptr[0] = '\n';
                break;
            case 'r':
                writeptr[0] = '\r';
                break;
            case 't':
                writeptr[0] = '\t';
                break;
            case 'b':
                writeptr[0] = '\b';
                break;
            case 'a':
                writeptr[0] = '\a';
                break;
            case 'f':
                writeptr[0] = '\f';
                break;
            case 'v':
                writeptr[0] = '\v';
                break;
            case '\\':
                writeptr[0] = '\\';
                break;
            case '"':
                writeptr[0] = '"';
                break;
            case 'x': { // for escaping hex digits
                u32 num = 0;

                // check if escape sequence is valid or not
                if (!is_hex(unescaped[2]) || !is_hex(unescaped[3])) {
                    xasm_error(E_INVALID_SYNTAX, (u32)__LINE__, (char*)__PRETTY_FUNCTION__, "hex sequence out of range");
                }

                // use only 2 digits
                if (sscanf(&unescaped[2], "%2x", &num) < 1) {
                    xasm_error(E_INVALID_SYNTAX, (u32)__LINE__, (char*)__PRETTY_FUNCTION__, "\\x used with no hex digits");
                }
                writeptr[0] = (char)num;
                unescaped += 2;
                break;
            }
            default:
                xasm_error(E_INVALID_SYNTAX, (u32)__LINE__, (char*)__PRETTY_FUNCTION__, "Unknown escape sequence : \"\\%c\"", unescaped[1]);
            };
            unescaped += 2;
            writeptr++;
            size++;
        } else {
            *writeptr = *unescaped;
            unescaped++;
            writeptr++;
            size++; // normal bytes just increment
        }
    }
    writeptr[0] = '\x00'; // terminate the string at the ending quote (")
    return size; // return the size of string or bytes
}
#pragma clang diagnostic pop

u32 resolve_number(char* num_s, u32* number)
{
    // detect base and return number
    if (num_s == NULL || number == NULL) {
        return E_ERR;
    }

    *number = 0;

    if (*num_s == '\x00' || *num_s == '\n') {
        return 0;
    }

    if (*num_s == '#') {
        // just in case if we get an
        // input starting with '#'
        num_s++;
    }

    char* z_ptr = strchr(num_s, '0');
    char* x_ptr = strchr(num_s, 'x');
    char* b_ptr = strchr(num_s, 'b');
    char* o_ptr = strchr(num_s, 'o');
    char* temp = NULL;

    // hexadecimal numbers
    if (z_ptr && x_ptr) {

        temp = ++x_ptr; // points to digits
        for (; *temp != '\x00'; temp++) {
            if (!is_hex(*temp)) {
                xasm_error(E_INVALID_IMMEDIATE, (u32)__LINE__ - 1, (char*)__PRETTY_FUNCTION__, "\"%c\" is not a valid hex digit", *temp);
                return E_ERR;
            }
        }

        *number = (u32)strtoul(x_ptr, NULL, 16);
        return E_OK;
    }

    // octal numbers
    if (z_ptr && o_ptr) {

        temp = ++o_ptr; // points to digits
        for (; *temp != '\x00'; temp++) {
            if (!is_digit(*temp)) {
                xasm_error(E_INVALID_IMMEDIATE, (u32)__LINE__ - 1, (char*)__PRETTY_FUNCTION__, "\"%c\" is not a valid digit", *temp);
                return E_ERR;
            }
        }
        *number = (u32)strtol(o_ptr, NULL, 8);
        return E_OK;
    }

    // binary numbers
    if (z_ptr && b_ptr) {

        temp = ++b_ptr; // points to digits
        for (; *temp != '\x00'; temp++) {
            if (!is_binary(*temp)) {
                xasm_error(E_INVALID_IMMEDIATE, (u32)__LINE__ - 1, (char*)__PRETTY_FUNCTION__, "\"%c\" is not a valid binary digit", *temp);
                return E_ERR;
            }
        }
        *number = (u32)strtol(b_ptr, NULL, 2);
        return E_OK;
    }

    // decimal numbers
    if (!x_ptr && !b_ptr && !o_ptr) {
        for (temp = num_s + 1; *temp != '\x00'; temp++) {
            if (!is_digit(*temp)) {
                xasm_error(E_INVALID_IMMEDIATE, (u32)__LINE__ - 1, (char*)__PRETTY_FUNCTION__, "\"%c\" is not a valid digit", *temp);
                return E_ERR;
            }
        }
        *number = (u32)strtol(num_s, NULL, 10);
        return E_OK;
    } else {
        // invalid base error out
        xasm_error(E_INVALID_IMMEDIATE, (u32)__LINE__ - 1, (char*)__PRETTY_FUNCTION__, "\"%s\" is not supported as immediate value", num_s);
        return E_ERR;
    }

    return E_ERR;
}

u32 xasm_resolve_number(char* num_s)
{
    u32 number = 0;
    if (resolve_number(num_s, &number) == E_ERR) {
        exit(E_ERR);
    }
    return number;
}

void xasm_info(char* msg, ...)
{

    char* error_msg = NULL;
    va_list valist;
    printf("[" KGRN "+" KNRM "] ");
    va_start(valist, msg);
    vprintf(msg, valist);
    va_end(valist);
}

void xasm_warn(char* msg, ...)
{

    char* error_msg = NULL;
    va_list valist;
    printf("[" KRED "!" KNRM "] ");
    va_start(valist, msg);
    vprintf(msg, valist);
    va_end(valist);
}

void xasm_error(u32 error_id, u32 line, char* func, char* msg, ...)
{

    char* error_msg = NULL;
    va_list valist;
    printf("[" KRED "!" KNRM "] ");
    va_start(valist, msg);
    vprintf(msg, valist);
    printf("\t");

    switch (error_id) {
    case E_INVALID_INPUTFILE:
        error_msg = "Cannot Open file";
        break;
    case E_INVALID_IMMEDIATE:
        error_msg = "Invalid Immediate value";
        break;
    case E_INVALID_SYNTAX:
        error_msg = "Invalid Syntax";
        break;
    case E_INVALID_OPCODE:
        error_msg = "Unknown Opcode";
        break;
    default:
        error_msg = "UNKNOWN ERROR";
        break;
    }

    va_end(valist);

    if (line && func) {
        printf("(%d:%s)\t", line, func);
    }

    if (msg == NULL) {
        puts(error_msg);
    } else {
        puts("");
    }
}

arg* init_arg()
{
    // initialize argument

    arg* x_arg = (arg*)malloc(sizeof(arg));
    memset(x_arg, 0, sizeof(arg));
    return x_arg;
}

u32 disp_arg(arg* x_arg)
{
    printf("arg_type: ");
    switch (x_arg->arg_type) {
    case ARG_REGD:
        printf("arg_register\t");
        printf("regid: 0x%x\n", x_arg->opt_regid);
        break;
    case ARG_IMMD:
        printf("arg_immediate\t");
        printf("value: 0x%.8x\n", x_arg->opt_value);
        break;
    case (ARG_PTRD | ARG_IMMD | ARG_REGD):
        printf("arg_pointer\t");
        printf("regid: 0x%x\t", x_arg->opt_regid);
        printf("offset: 0x%.8x\n", x_arg->opt_value);
        break;
    case (ARG_PTRD | ARG_IMMD):
        printf("arg_pointer\t");
        printf("offset: 0x%.8x\n", x_arg->opt_value);
        break;
    case (ARG_PTRD | ARG_REGD):
        printf("arg_pointer\t");
        printf("regid: 0x%x\n", x_arg->opt_regid);
        break;
    default:
        printf("arg_noarg\n");
        break;
    }
    return E_OK;
}

u32 fini_arg(arg* x_arg)
{
    // finish argument

    memset(x_arg, 0, sizeof(arg));
    free(x_arg);
    x_arg = NULL;
    return E_OK;
}
