//! tolken (tlk.h) v0.1.5
//!
//! https://github.com/simon-danielsson/tolken

/*
   Copyright © 2026 Simon Danielsson

   Permission is hereby granted, free of charge, to any person obtaining a copy
   of this software and associated documentation files, to deal in the Software
   without restriction, including without limitation the rights to use, copy,
   modify, merge, publish, distribute, sublicense, and/or sell copies of the
   Software, and to permit persons to whom the Software is furnished to do so,
   subject to the following conditions:

   The above copyright notice and this permission notice shall be included in
   all copies or substantial portions of the Software.

   THE SOFTWARE IS PROVIDED “AS IS”, WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
   IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
   FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
   AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
   LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
   OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
   SOFTWARE.
   */

#ifndef TLK_H_
#define TLK_H_

#include <ctype.h>
#include <errno.h>
#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>
#include <termios.h>
#include <unistd.h>

#endif // TLK_H_

#ifndef TLK_DEF
#define TLK_DEF

//-------------------------
// color      	fg    	bg
//-------------------------
// black      	30    	40
// red        	31    	41
// green      	32    	42
// yellow     	33    	43
// blue       	34    	44
// magenta    	35    	45
// cyan       	36    	46
// white      	37    	47
// default    	39    	49
// reset      	0     	0
//-------------------------

/// ansi color
typedef enum {
    BLACK,
    RED,
    GREEN,
    YELLOW,
    BLUE,
    MAGENTA,
    CYAN,
    WHITE,
    DEFAULT,
    RESET,
} Color;

/// style options
typedef struct {
    Color fg;
    Color bg;
    bool bold;
} Style;

/// coordinate column and row
typedef struct {
    unsigned int c;
    unsigned int r;
} Pos;

/// output of tlk_key()
typedef enum {
    CTRL_C,

    ARROW_UP,
    ARROW_DOWN,
    ARROW_LEFT,
    ARROW_RIGHT,

    SPACE,
    ESCAPE,
    ENTER,
    TAB,
    BACKSPACE,

    LOWER_A,
    LOWER_B,
    LOWER_C,
    LOWER_D,
    LOWER_E,
    LOWER_F,
    LOWER_G,
    LOWER_H,
    LOWER_I,
    LOWER_J,
    LOWER_K,
    LOWER_L,
    LOWER_M,
    LOWER_N,
    LOWER_O,
    LOWER_P,
    LOWER_Q,
    LOWER_R,
    LOWER_S,
    LOWER_T,
    LOWER_U,
    LOWER_V,
    LOWER_W,
    LOWER_X,
    LOWER_Y,
    LOWER_Z,

    UPPER_A,
    UPPER_B,
    UPPER_C,
    UPPER_D,
    UPPER_E,
    UPPER_F,
    UPPER_G,
    UPPER_H,
    UPPER_I,
    UPPER_J,
    UPPER_K,
    UPPER_L,
    UPPER_M,
    UPPER_N,
    UPPER_O,
    UPPER_P,
    UPPER_Q,
    UPPER_R,
    UPPER_S,
    UPPER_T,
    UPPER_U,
    UPPER_V,
    UPPER_W,
    UPPER_X,
    UPPER_Y,
    UPPER_Z,

    UNKNOWN,
    NONE
} Key;

/// restore screen to saved state
void tlk_screen_restore(void);

/// save screen state
void tlk_screen_save(void);

/// enable alternative buffer
void tlk_alt_buffer_enable(void);

/// disable alternative buffer
void tlk_alt_buffer_disable(void);

/// clear screen
void tlk_screen_clear(void);

/// reset color at cursor
void tlk_color_reset(void);

/// save cursor position
void tlk_cursor_save_pos(void);

/// restore cursor position
void tlk_cursor_restore_pos(void);

/// move cursor to home position (0,0)
void tlk_cursor_move_home(void);

void tlk_cursor_hide(void);
void tlk_cursor_show(void);
void tlk_cursor_mv(Pos *p);
void tlk_cursor_mv_down(int n);
void tlk_cursor_mv_up(int n);
void tlk_cursor_mv_right(int n);
void tlk_cursor_mv_left(int n);

/// get terminal size as Pos
Pos tlk_terminal_size();

/// flush draws to screen
void tlk_flush();

/// get key input as Key enum
Key tlk_key(void);

void tlk_disable_raw_mode();
void tlk_enable_raw_mode();

static unsigned int _tlk_color_to_code(Color col, bool fg) {
    if (col == RESET) {
        return 0;
    } else {
        unsigned int output = 0;
        switch (col) {
            case BLACK:
                output = 30;
                break;
            case RED:
                output = 31;
                break;
            case GREEN:
                output = 32;
                break;
            case YELLOW:
                output = 33;
                break;
            case BLUE:
                output = 34;
                break;
            case MAGENTA:
                output = 35;
                break;
            case CYAN:
                output = 36;
                break;
            case WHITE:
                output = 37;
                break;
            case DEFAULT:
            default:
                output = 39;
                break;
        }
        if (!fg) {
            output += 10;
        }
        return output;
    }
}

// drawing & style

/// set default style (to be used with function tlk_draw())
Style tlk_style_default(void);

/// clear line of the current cursor pos
void tlk_clear_current_line(void);

void tlk_clear_at_cursor_pos();

/// queues a new draw to screen (needs to be flushed)
void tlk_draw(const char *to_draw, Style c);

/// queue draw screen color
void tlk_screen_color(const Color c);

#endif // TLK_DEF
#ifdef TLK_IMPLEMENTATION

struct termios orig_termios;

static void tlk_die(const char *s) {
    perror(s);
    exit(1);
}

void tlk_flush(void) { fflush(stdout); }

void tlk_screen_restore(void) { puts("\x1B[?47l"); }
void tlk_screen_save(void) { puts("\x1B[?47h"); }
void tlk_alt_buffer_enable(void) { puts("\x1B[?1049h"); }
void tlk_alt_buffer_disable(void) { puts("\x1B[?1049l"); }
void tlk_screen_clear(void) { puts("\x1B[2J"); }

void tlk_color_reset(void) { puts("\x1B[0m"); }

void tlk_clear_current_line(void) { printf("\x1B[2K"); }

void tlk_cursor_save_pos(void) { puts("\x1B[s"); }
void tlk_cursor_restore_pos(void) { puts("\x1B[u"); }
void tlk_cursor_move_home(void) { puts("\x1B[H"); }
void tlk_cursor_hide(void) { puts("\x1B[?25l"); }
void tlk_cursor_show(void) { puts("\x1B[?25h"); }
void tlk_cursor_mv(Pos *p) { printf("\x1B[%d;%df", p->r, p->c); }

void tlk_cursor_mv_up(int n) { printf("\x1B[%dA", n); }
void tlk_cursor_mv_down(int n) { printf("\x1B[%dB", n); }
void tlk_cursor_mv_right(int n) { printf("\x1B[%dC", n); }
void tlk_cursor_mv_left(int n) { printf("\x1B[%dD", n); }

Pos tlk_terminal_size() {
    unsigned int rows = 0, cols = 0;
    char buf[32] = {0};

    write(STDOUT_FILENO, "\033[9999;9999H", 12);
    write(STDOUT_FILENO, "\033[6n", 4);

    read(STDIN_FILENO, buf, sizeof(buf) - 1);

    if (buf[0] == '\033' && buf[1] == '[') {
        sscanf(buf + 2, "%d;%dR", &rows, &cols);
    }
    return (Pos){.c = cols, .r = rows};
}

void tlk_disable_raw_mode() {
    if (tcsetattr(STDIN_FILENO, TCSAFLUSH, &orig_termios) == -1)
        tlk_die("tcsetattr");
}

void tlk_enable_raw_mode() {
    if (tcgetattr(STDIN_FILENO, &orig_termios) == -1)
        tlk_die("tcgetattr");
    atexit(tlk_disable_raw_mode);
    struct termios raw = orig_termios;
    raw.c_iflag &= ~(BRKINT | ICRNL | INPCK | ISTRIP | IXON);
    raw.c_oflag &= ~(OPOST);
    raw.c_cflag |= (CS8);
    raw.c_lflag &= ~(ECHO | ICANON | IEXTEN | ISIG);
    raw.c_cc[VMIN] = 0;
    raw.c_cc[VTIME] = 1;
    if (tcsetattr(STDIN_FILENO, TCSAFLUSH, &raw) == -1)
        tlk_die("tcsetattr");
}

Key tlk_key(void) {
    char c;

    size_t n = read(STDIN_FILENO, &c, 1);
    if (n == 0)
        return NONE;
    if (n < 0) {
        if (errno == EAGAIN || errno == EWOULDBLOCK)
            return NONE;
        return UNKNOWN;
    }
    // printf("key: 0x%02X\r\n", c);

    if (c >= 'a' && c <= 'z')
        return LOWER_A + (c - 'a');

    if (c >= 'A' && c <= 'Z')
        return UPPER_A + (c - 'A');

    switch (c) {
        case 0x03:
            return CTRL_C;
        case '\r':
        case '\n':
            return ENTER;
        case '\t':
            return TAB;
        case 0x20:
            return SPACE;
        case 0x7F:
            return BACKSPACE;

        case '\x1B': {
                         char seq[2];

                         if (read(STDIN_FILENO, &seq[0], 1) != 1)
                             return ESCAPE;

                         if (read(STDIN_FILENO, &seq[1], 1) != 1)
                             return ESCAPE;

                         if (seq[0] == '[') {
                             switch (seq[1]) {
                                 case 'A':
                                     return ARROW_UP;
                                 case 'B':
                                     return ARROW_DOWN;
                                 case 'C':
                                     return ARROW_RIGHT;
                                 case 'D':
                                     return ARROW_LEFT;
                             }
                         }

                         return UNKNOWN;
                     }

        default:
                     return UNKNOWN;
    }
}

Style tlk_style_default(void) {
    return (Style){.bg = DEFAULT, .fg = DEFAULT, .bold = false};
}

void tlk_clear_at_cursor_pos() { printf("\033[0m \033[0m"); }

void tlk_draw(const char *to_draw, Style c) {
    unsigned int bold = 22;
    if (c.bold) {
        bold = 1;
    }
    printf("\033[%d;%d;%dm%s\033[0m", bold, _tlk_color_to_code(c.fg, true),
            _tlk_color_to_code(c.bg, false), to_draw);
}

void tlk_screen_color(const Color c) {
    Pos size = tlk_terminal_size();

    if (size.c <= 0 || size.r <= 0) {
        tlk_disable_raw_mode();
        tlk_alt_buffer_disable();
        printf("negative terminal size");
        abort();
    }

    char row[size.c + 1];
    for (unsigned int i = 0; i < size.c; i++) {
        row[i] = ' ';
    }
    row[size.c] = '\0';

    Style style = {.bg = c, .fg = DEFAULT};

    for (unsigned int r = 0; r < size.r; r++) {
        tlk_cursor_mv(&(Pos){.c = 0, .r = r});
        tlk_draw(row, style);
    }

    tlk_cursor_move_home();
}

#endif
