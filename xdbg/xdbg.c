#include "commands.h"
#include <iface.h>
#include <stdlib.h>
#include <xdbg.h>

extern iface_state state;

void handle_sigint(int sigid)
{
    unload_binary(&state);
    putchar(10);
    xdbg_error("Exiting.\n");
    exit(E_ERR);
}

int main(int argc, char* argv[])
{

    if (argc != 2) {
        fprintf(stderr, "Usage: xdbg <bytecode>\n");
        exit(-1);
    }

    atexit((void (*)(void))handle_sigint);
    setbuf(stdin, 0);
    setbuf(stdout, 0);

    /* dbg_cpu(cpu, bin); */
    xdbg_iface(&state, argv[1]);
    return E_OK;
}
