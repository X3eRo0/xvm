#ifndef XDBG_H
#define XDBG_H

#include <cpu.h>
#include <iface.h>
#include <cstring.h>

#define REPL_BUFFER_SZ 0x100
#define MAX_PROMPT_SIZE 0x100

typedef struct repl_state_t {
    u32 cursor;
    s_str buffer;
    char* prompt;
} repl_state;

repl_state* repl_init(char* prompt);
void repl_fini(repl_state* rs);
u32 repl_clear(repl_state* rs);
u32 repl_insert(repl_state* rs, u8 chr);
u32 repl_backspace(repl_state* rs);
u32 repl_delete(repl_state* rs);
u32 repl_readline(repl_state* rs, char* buffer, u32 len);
u32 repl_handle_keys(repl_state* rs, u64 ch);
void repl_render(repl_state* rs);
void repl_move_left(repl_state* rs);
void repl_move_right(repl_state* rs);
void repl_move_left_word(repl_state* rs);
void repl_move_right_word(repl_state* rs);
void repl_move_home(repl_state* rs);
void repl_move_end(repl_state* rs);
void repl_fix_cursor(repl_state* rs);
void repl_give_autosuggestion(repl_state* rs);
void tty_write(char* buffer);
void tty_atexit(void);
int tty_reset(void);
void tty_raw(void);
void unload_binary(iface_state* state);
void load_binary(iface_state* state, const char* filename);
void handle_signals(iface_state* state);
void dbg_cpu(iface_state* state);

#endif // !XDBG_H
