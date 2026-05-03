//! tolken (tlk.h) v0.4.6
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
#include <stdio.h>
#include <stdlib.h>
#include <termios.h>
#include <unistd.h>

#endif // TLK_H_

#ifndef TLK_DEF
#define TLK_DEF

/// coordinate column and row
typedef struct {
  unsigned int c;
  unsigned int r;
} TPos;

void tlk_screen_restore(void);
void tlk_screen_save(void);
void tlk_alt_buffer_enable(void);
void tlk_alt_buffer_disable(void);
void tlk_screen_clear(void);

void tlk_color_reset(void);

void tlk_line_clear_current(void);

void tlk_cursor_save_pos(void);
void tlk_cursor_restore_pos(void);
void tlk_cursor_move_home(void);
void tlk_cursor_hide(void);
void tlk_cursor_show(void);

TPos tlk_terminal_size();

void tlk_disable_raw_mode();
void tlk_enable_raw_mode();

#endif // TLK_DEF

#ifdef TLK_IMPLEMENTATION

struct termios orig_termios;

static void tlk_die(const char *s) {
  perror(s);
  exit(1);
}

void tlk_screen_restore(void) { puts("\x1B[?47l"); }
void tlk_screen_save(void) { puts("\x1B[?47h"); }
void tlk_alt_buffer_enable(void) { puts("\x1B[?1049h"); }
void tlk_alt_buffer_disable(void) { puts("\x1B[?1049l"); }
void tlk_screen_clear(void) { puts("\x1B[2J"); }

void tlk_color_reset(void) { puts("\x1B[0m"); }

void tlk_line_clear_current(void) { puts("\x1B[2K"); }

void tlk_cursor_save_pos(void) { puts("\x1B[s"); }
void tlk_cursor_restore_pos(void) { puts("\x1B[u"); }
void tlk_cursor_move_home(void) { puts("\x1B[H"); }
void tlk_cursor_hide(void) { puts("\x1B[?25l"); }
void tlk_cursor_show(void) { puts("\x1B[?25h"); }

TPos tlk_terminal_size() {
  int rows = 0, cols = 0;
  char buf[32] = {0};

  write(STDOUT_FILENO, "\033[9999;9999H", 12);
  write(STDOUT_FILENO, "\033[6n", 4);

  read(STDIN_FILENO, buf, sizeof(buf) - 1);

  if (buf[0] == '\033' && buf[1] == '[') {
    sscanf(buf + 2, "%d;%dR", &rows, &cols);
  }
  return (TPos){.c = cols, .r = rows};
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

#endif
