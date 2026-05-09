//! main entry point of program
#include <stddef.h>
#define ANA_IMPLEMENTATION
#define TLK_IMPLEMENTATION
#include "../tlk.h"
#include "env.h"

void setup(void) {
    tlk_cursor_save_pos();
    tlk_screen_save();
    tlk_cursor_hide();
    tlk_enable_raw_mode();
    tlk_screen_clear();
}

void cleanup(void) {
    tlk_screen_clear();
    tlk_disable_raw_mode();
    tlk_alt_buffer_disable();
    tlk_screen_restore();
    tlk_cursor_show();
    tlk_color_reset();
    tlk_cursor_restore_pos();
}

typedef struct {
    Key key;
    Pos size;
    bool should_quit;
    char *entry_buffer;
    size_t *entry_buffer_count;
} TlkApp;

void TlkApp_init(TlkApp *app) {
    app->entry_buffer = (char *)malloc(sizeof(char) + 1);
    app->entry_buffer_count = 0;
    app->should_quit = false;
    app->key = NONE;
    app->size = tlk_terminal_size();
}

void TlkApp_push_char_to_buff(TlkApp *app, char c) {
    char *tmp =
        realloc(app->entry_buffer, *(app->entry_buffer_count + 1) * sizeof(char));
    if (!tmp) {
        ERROR("Allocation failure");
        return;
    }
    app->entry_buffer = tmp;
    app->entry_buffer[*app->entry_buffer_count] = c;
    app->entry_buffer_count++;
}

void TlkApp_remove_last_char_from_buff(TlkApp *app) {
    char *tmp =
        realloc(app->entry_buffer, *(app->entry_buffer_count - 1) * sizeof(char));
    if (!tmp) {
        ERROR("Allocation failure");
        return;
    }
    app->entry_buffer = tmp;
    // app->entry_buffer[*app->entry_buffer_count] = c;
    app->entry_buffer_count--;
}

int main(void) {
    setup();
    TlkApp app;
    TlkApp_init(&app);
    tlk_screen_clear();
    tlk_screen_color(GREEN);
    bool quit = false;

    bool typing = false;
    while (!quit) {
        // update
        {
            app.size = tlk_terminal_size();
            app.key = tlk_key();
        }

        if (app.key == ENTER) {
            typing = true;
            tlk_cursor_move_home();
        }

        while (typing) {
            char *c;
            while (c[strlen(c)] != '\n') {
                bool n = read(STDIN_FILENO, &c, 1);
                // TlkApp_push_char_to_buff(&app, c);

                if (n) {
                    if (c == '\n' || c == '\r') {
                        typing = false;
                        break;
                    }
                    if (c == 127) { // 127 = backspace
                        tlk_cursor_move_home();
                        tlk_clear_current_line();
                        TlkApp_remove_last_char_from_buff(&app);
                        printf("%s", app.entry_buffer);

                        fflush(stdout);
                        continue;
                    }
                    printf("%c", c);
                    fflush(stdout);
                }
            }
        }

        // controls
        {
            if (app.key == ESCAPE || app.key == CTRL_C) {
                quit = true;
            }
        }

        // draw
        {
            tlk_cursor_mv(&(Pos){.c = app.size.c / 2, .r = app.size.r / 2});

            if (app.key == ARROW_LEFT) {
                tlk_clear_current_line();
                tlk_draw("LEFT", tlk_style_default());
            } else if (app.key == ARROW_RIGHT) {
                tlk_clear_current_line();
                tlk_draw("RIGHT", (Style){.fg = BLACK, .bg = MAGENTA, .bold = true});
            } else if (app.key == ARROW_UP) {
                // tlk_clear_current_line();
                // printf("\x1b[1;31mUP");
                tlk_clear_at_cursor_pos();
            } else if (app.key == ARROW_DOWN) {
                tlk_clear_current_line();
                printf("\x1b[1;31mDOWN");
            } else if (app.key == ENTER) {
                tlk_clear_current_line();
                printf("\x1b[1;31mENTER");
            } else if (app.key == TAB) {
                tlk_clear_current_line();
                printf("\x1b[1;31mTAB");
            } else if (app.key == BACKSPACE) {
                tlk_clear_current_line();
                printf("\x1b[1;31mBACKSPACE");
            } else if (app.key == SPACE) {
                tlk_clear_current_line();
                printf("\x1b[1;31mSPACE");
            }
        }

        // flush
        {
            tlk_flush();
        }
    }
    cleanup();

    return 0;
}
