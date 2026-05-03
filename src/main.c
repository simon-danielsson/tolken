//! main entry point of program
#define ANA_IMPLEMENTATION
#define TLK_IMPLEMENTATION
#include "../tlk.h"
#include "env.h"

void setup(void) {
    tlk_screen_save();
    tlk_cursor_save_pos();
    tlk_cursor_hide();
    tlk_enable_raw_mode();
    tlk_screen_clear();
}

void cleanup(void) {
    tlk_disable_raw_mode();
    tlk_alt_buffer_disable();
    tlk_screen_restore();
    tlk_cursor_restore_pos();
    tlk_cursor_show();
    tlk_color_reset();
}

void tlk_cursor_move(TPos *p) {
    tlk_cursor_move_home();
    printf("\x1B[%d;%df", p->r, p->c);
}

typedef struct {
    TPos term_size;
} TlkApp;

void TlkApp_init(TlkApp *app) { app->term_size = tlk_terminal_size(); }

int main(void) {
    setup();
    TlkApp app;
    TlkApp_init(&app);
    tlk_cursor_move(&(TPos){.c = 5, .r = 10});
    tlk_screen_clear();
    puts("\x1b[1;31mHello");
    al_sleep(1);
    tlk_cursor_move(&(TPos){.c = 7, .r = 13});
    puts("\x1b[2;37;32mWorld");
    tlk_color_reset();
    al_sleep(1);
    tlk_screen_clear();
    cleanup();
    printf("\nc: %d, r: %d\n", app.term_size.c, app.term_size.r);
    return 0;
}
