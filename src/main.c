//! main entry point of program
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
} TlkApp;

void TlkApp_init(TlkApp *app) {
    app->should_quit = false;
    app->key = NONE;
    app->size = tlk_terminal_size();
}

int main(void) {
    setup();
    TlkApp app;
    TlkApp_init(&app);
    tlk_screen_clear();
    tlk_screen_color(GREEN);
    bool quit = false;
    while (!quit) {
        // update
        {
            app.size = tlk_terminal_size();
            app.key = tlk_key();
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
