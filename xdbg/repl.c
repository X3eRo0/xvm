/* Set terminal (tty) into "raw" mode: no line or other processing done
   Terminal handling documentation:
       curses(3X)  - screen handling library.
       tput(1)     - shell based terminal handling.
       terminfo(4) - SYS V terminal database.
       termcap     - BSD terminal database. Obsoleted by above.
       termio(7I)  - terminal interface (ioctl(2) - I/O control).
       termios(3)  - preferred terminal interface (tc* - terminal control).
*/

#include "cstring.h"
#include <ascii.h>
#include <ctype.h>
#include <iface.h>
#include <stdio.h>
#include <stdlib.h>
#include <termios.h>
#include <unistd.h>
#include <xdbg.h>

void tty_atexit(void);
int tty_reset(void);
void tty_raw(void);
int screenio(void);
void xdbg_error(char* mess);

static struct termios orig_termios; /* TERMinal I/O Structure */
static int ttyfd = STDIN_FILENO; /* STDIN_FILENO is 0 by default */

repl_state* repl_init(char* prompt)
{
    /* check that input is from a tty */
    if (!isatty(ttyfd))
        xdbg_error("not on a tty");

    /* store current tty settings in orig_termios */
    if (tcgetattr(ttyfd, &orig_termios) < 0)
        xdbg_error("can't get tty settings");

    /* register the tty reset with the exit handler */
    if (atexit(tty_atexit) != 0)
        xdbg_error("atexit: can't register tty reset");

    repl_state* rs = (repl_state*)calloc(1, sizeof(repl_state));
    rs->buffer = s_str_create();
    rs->cursor = 0;
    rs->prompt = strdup(prompt);

    // tty_raw(); /* put tty in raw mode */
    // screenio(); /* run application code */
    return rs; /* tty_atexit will restore terminal */
}

void repl_fini(repl_state* rs)
{
    if (rs == NULL) {
        xdbg_error("Repl State Corrupted");
        return;
    }

    s_str_destroy(&rs->buffer);
    free(rs->prompt);
    memset(rs, 0, sizeof(repl_state));
    free(rs);
    return;
}

u32 repl_clear(repl_state* rs)
{
    if (rs == NULL) {
        xdbg_error("Repl State Corrupted");
        return E_ERR;
    }
    rs->cursor = 0;
    s_str_clear(&rs->buffer);
    return E_OK;
}

u32 repl_insert(repl_state* rs, u8 chr)
{
    if (rs == NULL) {
        xdbg_error("Repl State Corrupted");
        return E_ERR;
    }

    if (chr == 0) {
        return E_OK;
    }

    s_str_insert_char(&rs->buffer, rs->cursor++, chr);
    return E_OK;
}

u32 repl_backspace(repl_state* rs)
{
    if (rs->cursor > 0) {
        s_str_erase(&rs->buffer, rs->cursor - 1, 1);
        rs->cursor--;
    }

    return E_OK;
}

u32 repl_delete(repl_state* rs)
{
    if (rs->cursor >= 0) {
        s_str_erase(&rs->buffer, rs->cursor, 1);
    }

    return E_OK;
}

void repl_render(repl_state* rs)
{
    tty_write(DELETE_LINE);
    tty_write(MOVE_HOME);
    tty_write(rs->prompt);
    tty_write((char*)s_str_c_str(&rs->buffer));
    repl_fix_cursor(rs);
}

/* exit handler for tty reset */
void tty_atexit(void) /* NOTE: If the program terminates due to a signal   */
{ /* this code will not run.  This is for exit()'s     */
    tty_reset(); /* only.  For resetting the terminal after a signal, */
} /* a signal handler which calls tty_reset is needed. */

/* reset tty - useful also for restoring the terminal when this process
   wishes to temporarily relinquish the tty
*/

void tty_write(char* buffer)
{
    write(STDOUT_FILENO, buffer, strlen(buffer));
}

void repl_move_left(repl_state* rs)
{
    if (rs->cursor > 0) {
        tty_write(MOVE_LEFT);
        rs->cursor--;
    }
}

void repl_move_right(repl_state* rs)
{
    if (rs->cursor < s_str_length(&rs->buffer)) {
        tty_write(MOVE_RIGHT);
        rs->cursor++;
    }
}

void repl_move_left_word(repl_state* rs)
{
    u32 len = s_str_length(&rs->buffer);
    while (rs->cursor > 0 && rs->cursor <= len && !isalpha(s_str_at(&rs->buffer, rs->cursor - 1))) {
        tty_write(MOVE_LEFT);
        rs->cursor--;
    }

    while (rs->cursor > 0 && rs->cursor <= len && isalpha(s_str_at(&rs->buffer, rs->cursor - 1))) {
        tty_write(MOVE_LEFT);
        rs->cursor--;
    }
}

void repl_move_right_word(repl_state* rs)
{
    u32 len = s_str_length(&rs->buffer);
    while (rs->cursor < len && !isalpha(s_str_at(&rs->buffer, rs->cursor))) {
        tty_write(MOVE_RIGHT);
        rs->cursor++;
    }

    while (rs->cursor < len && isalpha(s_str_at(&rs->buffer, rs->cursor))) {
        tty_write(MOVE_RIGHT);
        rs->cursor++;
    }
}

void repl_move_home(repl_state* rs)
{
    rs->cursor = 0;
}

void repl_move_end(repl_state* rs)
{
    rs->cursor = s_str_length(&rs->buffer);
}

void repl_fix_cursor(repl_state* rs)
{
    char buffer[100] = { 0 };
    snprintf(buffer, sizeof(buffer), MOVE_TO, (int)(rs->cursor + 1 + strlen(rs->prompt)));
    tty_write(buffer);
}

void repl_give_autosuggestion(repl_state* rs)
{
    // find word to complete
    u32 lw_idx = s_str_length(&rs->buffer) - strlen(s_str_last_word(&rs->buffer));
    for (u32 i = 0; cmds[i].cmd != NULL; i++) {
        if (!strncmp(cmds[i].cmd, s_str_last_word(&rs->buffer), strlen(s_str_last_word(&rs->buffer)))) {
            // insert string
            s_str_remove_last_word(&rs->buffer);
            s_str_insert_c_str(&rs->buffer, lw_idx, cmds[i].cmd);
            rs->cursor = s_str_length(&rs->buffer);
            break;
        }
    }
}

int tty_reset(void)
{
    /* flush and reset */
    if (tcsetattr(ttyfd, TCSAFLUSH, &orig_termios) < 0)
        return -1;
    return 0;
}

/* put terminal in raw mode - see termio(7I) for modes */
void tty_raw(void)
{
    struct termios raw;

    raw = orig_termios; /* copy original and then modify below */

    /* input modes - clear indicated ones giving: no break, no CR to NL,
       no parity check, no strip char, no start/stop output (sic) control */
    raw.c_iflag &= ~(BRKINT | ICRNL | INPCK | ISTRIP | IXON | ECHO);

    /* output modes - clear giving: no post processing such as NL to CR+NL */
    raw.c_oflag &= ~(OPOST);

    /* control modes - set 8 bit chars */
    raw.c_cflag |= (CS8);

    /* local modes - clear giving: echoing off, canonical off (no erase with
       backspace, ^U,...),  no extended functions, no signal chars (^Z,^C) */
    raw.c_lflag &= ~(ECHO | ICANON | IEXTEN | ISIG);

    /* control chars - set return condition: min number of bytes and timer */
    raw.c_cc[VMIN] = 5;
    raw.c_cc[VTIME] = 8; /* after 5 bytes or .8 seconds
                            after first byte seen      */
    raw.c_cc[VMIN] = 0;
    raw.c_cc[VTIME] = 0; /* immediate - anything       */
    raw.c_cc[VMIN] = 2;
    raw.c_cc[VTIME] = 0; /* after two bytes, no timer  */
    raw.c_cc[VMIN] = 0;
    raw.c_cc[VTIME] = 8; /* after a byte or .8 seconds */

    /* put terminal in raw mode after flushing */
    if (tcsetattr(ttyfd, TCSAFLUSH, &raw) < 0)
        xdbg_error("can't set raw mode");
}

/* Read and write from tty - this is just toy code!!
   Prints T on timeout, quits on q input, prints Z if z input, goes up
   if u input, prints * for any other input character
*/

u32 repl_handle_keys(repl_state* rs, u64 ch)
{
    switch (ch) {
    case FF: {
        // clear screen
        tty_write(CLEAR_SCREEN);
        tty_write(MOVE_TOPLEFT);
        break;
    }
    case CR: {
        // return the string
        tty_write("\r\n");
        return E_ERR;
    }
    case DEL: {
        repl_backspace(rs);
        break;
    }
    case DEL_KEY_LONG: {
        repl_delete(rs);
        break;
    }
    case MOVE_UP_LONG:
    case MOVE_DOWN_LONG: {
        break;
    }
    case MOVE_LEFT_LONG: {
        repl_move_left(rs);
        break;
    }
    case MOVE_RIGHT_LONG: {
        repl_move_right(rs);
        break;
    }
    case CTRL_LEFT_LONG: {
        repl_move_left_word(rs);
        break;
    }
    case CTRL_RIGHT_LONG: {
        repl_move_right_word(rs);
        break;
    }
    case MOVE_HOME_LONG: {
        repl_move_home(rs);
        break;
    }
    case MOVE_END_LONG: {
        repl_move_end(rs);
        break;
    }
    case ETX: {
        exit(0);
    }
    case TAB: {
        // to autosuggestion
        repl_give_autosuggestion(rs);
        break;
    }
    default: {
        repl_insert(rs, (u8)ch);
        break;
    }
    }

    return E_OK;
}

u32 repl_readline(repl_state* rs, char* buffer, u32 len)
{
    tty_raw();
    u32 ret = 0;
    u64 ch = 0;
    while (!ret) {
        repl_render(rs);
        read(STDIN_FILENO, &ch, 8);
        ret = repl_handle_keys(rs, ch);
        ch = 0;
    }
    tty_reset();
    strncpy(buffer, s_str_c_str(&rs->buffer), len);
    return E_OK;
}
