//! ana.h v0.4.5
//!
//! https://github.com/simon-danielsson/analib.h

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

#ifndef ANALIB_H_
#define ANALIB_H_

#include <ctype.h>
#include <limits.h>
#include <pwd.h>
#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/stat.h>
#include <unistd.h>

#ifndef ANALIB_DEF
#define ANALIB_DEF
#endif // ANALIB_DEF

// INTERNAL DEFINITIONS
// =============================================================================

typedef struct {
  char *msg_col;
  int line;
  const char *function;
  const char *file;
  char *type;
  const char *msg;
} _al_db_type;

// generic label builder for debug functions
static inline void _al_db_lbl(const char *label, char *header,
                              int header_size) {
  if (header_size <= 0)
    return;

  int written = snprintf(header, header_size, " %s ", label);
  if (written < 0 || written >= header_size)
    return;

  int remaining = header_size - written - 1; // leave room for '\0'
  memset(header + written, ' ', remaining);
  header[written + remaining] = '\0';
}

#define _al_reset_clr "\033[0m"
// generic msg call for debug functions
static inline void _al_db_msg(_al_db_type *t) {
  char label[9];
  _al_db_lbl(t->type, label, 9);

  char space[11 + 1];
  memset(space, ' ', 11);
  space[11] = '\0';

  fprintf(stderr, "%s%s %s%s:%d (%s)%s%s\n", t->msg_col, t->type, _al_reset_clr,
          t->file, t->line, t->function, " ", t->msg);
}

static inline int _al_min_int(int a, int b) { return a < b ? a : b; }
static inline int _al_max_int(int a, int b) { return a > b ? a : b; }
static inline double _al_min_double(double a, double b) {
  return a < b ? a : b;
}
static inline double _al_max_double(double a, double b) {
  return a > b ? a : b;
}

// PUBLIC API
// =============================================================================

// checks if the input str contains nothing
bool al_is_empty_or_whitespace(const char *str);

// remove special characters form str in place
void al_rem_spec_chars(char *str);

// checks if param *str contains param *word
bool al_contains(const char *str, const char *word);

// checks if param *str starts with param *word
bool al_starts_with(const char *str, const char *word);

// checks if *str starts *word, ignoring leading whitespace
bool al_starts_with_no_w(const char *line, const char *word);

// strip start and end of str
char *al_strip_start_end(char *s);

// helper for duplicating strings
char *al_strdup(const char *s);

// joins lines with delimiter
char *al_join_lines(char **lines, int n, char *delim);

// returns true if input is a valid path
bool al_is_valid_path(const char *s);

// strips input string to the last filename/dir name
char *al_strip_path(char *file_name);

// converts characters of input string to uppercase
char *al_str_to_upper(const char *str);

// remove first n characters from string with memmove
void al_remove_first_n(char *c, int n);

#define _al_log_clr "\033[34m"
#define _al_assert_clr "\033[31m"
#define _al_error_clr "\033[31m"
#define _al_assert_succ_clr "\033[32m"
#define _al_todo_clr "\033[33m"

// formatted assert message
// prints to stderr regardless of condition
// does not abort on do_abort=true if condition was true
#define ASSERT(cond, do_abort)                                                 \
  do {                                                                         \
    char msg[32];                                                              \
    char msg_col[32];                                                          \
    if (!(cond)) {                                                             \
      snprintf(msg, sizeof(msg), "failure -> %s", #cond);                      \
      snprintf(msg_col, sizeof(msg_col), "%s", _al_assert_clr);                \
    } else {                                                                   \
      snprintf(msg, sizeof(msg), "success -> %s", #cond);                      \
      snprintf(msg_col, sizeof(msg_col), "%s", _al_assert_succ_clr);           \
    }                                                                          \
    _al_db_msg(&(_al_db_type){.msg_col = msg_col,                              \
                              .line = __LINE__,                                \
                              .function = __func__,                            \
                              .file = __FILE_NAME__,                           \
                              .type = "ASSERT",                                \
                              .msg = msg});                                    \
    if (!(cond) && (do_abort)) {                                               \
      abort();                                                                 \
    }                                                                          \
  } while (0)

// formatted log message
#define LOG(fmt, ...)                                                          \
  do {                                                                         \
    char msg[128];                                                             \
    snprintf(msg, sizeof(msg), fmt __VA_OPT__(, ) __VA_ARGS__);                \
    _al_db_msg(&(_al_db_type){.msg_col = _al_log_clr,                          \
                              .line = __LINE__,                                \
                              .function = __func__,                            \
                              .file = __FILE_NAME__,                           \
                              .type = "INFO",                                  \
                              .msg = msg});                                    \
  } while (0)

// formatted log message
#define ERROR(fmt, ...)                                                        \
  do {                                                                         \
    char msg[128];                                                             \
    snprintf(msg, sizeof(msg), fmt __VA_OPT__(, ) __VA_ARGS__);                \
    _al_db_msg(&(_al_db_type){.msg_col = _al_error_clr,                        \
                              .line = __LINE__,                                \
                              .function = __func__,                            \
                              .file = __FILE_NAME__,                           \
                              .type = "ERROR",                                 \
                              .msg = msg});                                    \
    exit(1);                                                                   \
  } while (0)

// rust-like formatted todo message that aborts the program if reached

#define IMPL(fmt, ...)                                                         \
  do {                                                                         \
    char msg[128];                                                             \
    snprintf(msg, sizeof(msg), fmt __VA_OPT__(, ) __VA_ARGS__);                \
    char msg2[128];                                                            \
    snprintf(msg2, sizeof(msg2), "Not implemented -> %s", msg);                \
    _al_db_msg(&(_al_db_type){.msg_col = _al_todo_clr,                         \
                              .line = __LINE__,                                \
                              .function = __func__,                            \
                              .file = __FILE_NAME__,                           \
                              .type = "IMPL",                                  \
                              .msg = msg2});                                   \
    abort();                                                                   \
  } while (0)

#endif // ANALIB_H_

// IMPLEMENTATIONS
// =============================================================================

#ifdef ANALIB_IMPLEMENTATION

/// checks if the input str contains nothing
bool al_is_empty_or_whitespace(const char *str) {
  if (str == NULL)
    return true;
  while (*str) {
    if (!isspace((unsigned char)*str)) {
      return false;
    }
    str++;
  }
  return true;
}

/// remove special characters form str in place
void al_rem_spec_chars(char *str) {
  char *dst = str;

  while (*str) {
    if (isalnum((unsigned char)*str) || *str == '_') {
      *dst++ = *str;
    }
    str++;
  }

  *dst = '\0';
}

/// checks if param *str contains param *word
bool al_contains(const char *str, const char *word) {
  return strstr(str, word) != NULL;
}

// checks if param *str starts with param *word
bool al_starts_with(const char *str, const char *word) {
  size_t len_str = strlen(str);
  size_t len_word = strlen(word);

  if (len_word > len_str)
    return false;
  return strncmp(str, word, len_word) == 0;
}

/// checks if *str starts *word, ignoring leading whitespace
bool al_starts_with_no_w(const char *line, const char *word) {
  if (!line || !word)
    return false;
  // skip leading whitespace
  while (*line && isspace((unsigned char)*line)) {
    line++;
  }
  size_t len = strlen(word);
  if (strncmp(line, word, len) != 0) {
    return false;
  }
  return true;
}

/// helper for duplicating strings
char *al_strdup(const char *s) {
  if (!s)
    return NULL;
  char *out = malloc(strlen(s) + 1);
  if (!out) {
    ERROR("Memory allocation failed");
    return NULL;
  }
  strcpy(out, s);
  return out;
}

/// joins lines with delimiter
/// @param input array of *char
/// @param n count of array
/// @param delimiter between each line
/// @return a single concatenated string
char *al_join_lines(char **lines, int n, char *delim) {
  size_t total = 1; // for '\0'

  for (int i = 0; i < n; i++) {
    if (lines[i]) {
      total += (strlen(lines[i]) + strlen(delim));
    }
  }

  char *out = malloc(total);
  if (!out) {
    ERROR("Memory allocation failed");
    return NULL;
  }

  out[0] = '\0';

  for (int i = 0; i < n; i++) {
    if (lines[i]) {
      strcat(out, lines[i]);
      strcat(out, delim);
      lines[i] = NULL;
      free(lines[i]);
    }
  }

  return out;
}

/// returns true if input is a valid path
/// @param relative or absolute path to file or dir
bool al_is_valid_path(const char *s) {
  struct stat path_stat;

  if (stat(s, &path_stat) != 0) {
    // path does not exist or is not accessible
    return false;
  }

  // path is:
  if (S_ISREG(path_stat.st_mode)) {
    // regular file
    return true;
  } else if (S_ISDIR(path_stat.st_mode)) {
    // directory
    return true;
  } else {
    // something else (symlink, socket etc.)
    return false;
  }
  return true;
}

/// strips input string to the last filename/dir name
/// @param path
/// @important unix '/' paths only
char *al_strip_path(char *file_name) {
  char output[200] = {0};
  int counter = strlen(file_name);
  while (true) {
    if (file_name[counter] != '/') {
      strcpy(output, &file_name[counter]);
      counter--;
    } else {
      break;
    }
  }
  return al_strdup(output);
}

/// strip start and end of str
char *al_strip_start_end(char *s) {
  char op[strlen(s)];
  op[0] = '\0';
  while (*s && isspace((unsigned char)*s)) {
    s++;
  }
  int len = strlen(s);
  while (true) {
    len--;
    if (!isspace((unsigned char)s[len])) {
      // printf("%c\n", s[len]);
      break;
    }
  }
  strncat(op, s, len + 1);
  return al_strdup(op);
}

/// remove first n characters from string with memmove
/// @param string
/// @param n len to remove
void al_remove_first_n(char *c, int n) {
  int len = strlen(c);
  if (n >= len) {
    c[0] = '\0';
    return;
  }
  memmove(c, c + n, len - n + 1); // +1 for null terminator
}

/// converts characters of input string to uppercase
/// @param lowercase str
/// @return uppercase str
char *al_str_to_upper(const char *str) {
  int i = 0;
  char temp[124];
  temp[0] = '\0';
  while (str[i] != '\0') {
    temp[i] = toupper(str[i]); // Convert character to uppercase
    i++;
  }
  return al_strdup(temp);
}

#endif // ANALIB_IMPLEMENTATION
