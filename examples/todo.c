#ifdef TODO

#include <stdio.h>

#define TLK_IMPLEMENTATION
#include "../tlk.h"
#define ANA_IMPLEMENTATION
#include "../src/libs/ana.h"

typedef enum { DONE, NOT_DONE } Status;

typedef struct {
    char *content;
    Status s;
} Item;

typedef struct {
    Key k;
    bool should_quit;
    Pos size;
    bool redraw;
    Item *items;
    size_t items_amt;
    int item_sel;
    bool item_del;
    bool item_toggle;
    bool in_entry;
    char *entry_buf;
    size_t entry_buf_amt;
} App;

void setup(void);
void cleanup(void);
void controls(App *a);
void App_init(App *a);
void add_new_item(App *a);
void App_delete_item(App *a);
void App_push_item(App *a, Item *i);
void App_push_entry_char(App *a, char c);
void App_remove_last_entry_char(App *a);
void App_clear_entry_buff(App *a);
void draw_header(unsigned int *col);
void draw_controls(Pos *term_size);
void draw_items(App *a);
void toggle_item_status(App *a);
void clear_items_section(App *a);

int main() {
    setup();
    App a;
    App_init(&a);
    Pos term_size;
    while (!a.should_quit) {

        // update
        {
            term_size = tlk_terminal_size();
            controls(&a);
            if (term_size.c != a.size.c || term_size.r != a.size.r) {
                a.size = term_size;
                a.redraw = true;
            }
            if (a.item_del) {
                App_delete_item(&a);
            }
            if (a.item_toggle) {
                toggle_item_status(&a);
                a.item_toggle = false;
            }
        }

        // draw
        {
            if (a.item_del) {
                clear_items_section(&a);
                a.item_del = false;
            }
            if (a.redraw) {
                draw_header(&a.size.c);
                draw_controls(&a.size);
                a.redraw = false;
            }
            while (a.in_entry) {
                add_new_item(&a);
            }
            if (a.items_amt > 0)
                draw_items(&a);
        }

        tlk_flush();
    }

    cleanup();
    return 0;
}

void clear_items_section(App *a) {
    for (unsigned int rows = 2; rows < (a->size.r - 4); rows++) {
        tlk_cursor_mv(&(Pos){.c = 0, .r = rows});
        tlk_clear_current_line();
    }
}

void draw_controls(Pos *term_size) {
    unsigned int center_col = term_size->c / 2;
    unsigned int row = term_size->r - 1;
    tlk_cursor_mv(&(Pos){.c = 0, .r = row - 1});
    // clear before draw
    tlk_clear_current_line();
    tlk_cursor_mv(&(Pos){.c = center_col - (22 / 2), .r = row - 1});
    // draw first line
    Style st = {.fg = BLUE, .bold = true};
    tlk_draw("q ", st);
    tlk_draw("quit •", tlk_style_default());
    tlk_draw(" n ", st);
    tlk_draw("new •", tlk_style_default());
    tlk_draw(" d ", st);
    tlk_draw("del", tlk_style_default());
    // clear before draw
    tlk_cursor_mv(&(Pos){.c = 0, .r = row});
    tlk_clear_current_line();
    // draw second line
    tlk_cursor_mv(&(Pos){.c = center_col - (32 / 2), .r = row});
    tlk_draw("t ", st);
    tlk_draw("toggle •", tlk_style_default());
    tlk_draw(" ↓/↑ ", st);
    tlk_draw("nav •", tlk_style_default());
    tlk_draw(" esc ", st);
    tlk_draw("cancel", tlk_style_default());
    // clear last line
    tlk_cursor_mv(&(Pos){.c = 0, .r = row + 1});
    tlk_clear_current_line();
};

void App_delete_item(App *a) {
    for (size_t k = 0; k < a->items_amt; k++) {
        if ((int)k == a->item_sel) {
            for (size_t i = k; i < a->items_amt - 1; i++) {
                (a->items)[i] = (a->items)[i + 1];
            }
            a->item_sel = 0;
            a->items_amt--;
            if (a->items_amt == 0) {
                free(a->items);
                a->items = NULL;
            } else {
                Item *tmp = realloc(a->items, (a->items_amt) * sizeof(Item));
                if (tmp != NULL) {
                    a->items = tmp; // keep old pointer if realloc fails
                }
            }
            return;
        }
    }
}

void add_new_item(App *a) {
    char c = '\0';
    bool cancel = false;
    while (true) {
        bool n = read(STDIN_FILENO, &c, 1);

        if (n) {
            if (c == 127) { // 127 = backspace
                App_remove_last_entry_char(a);
                continue;
            }
            if (c == '\x1B') {
                cancel = true;
                break;
            }
            if (c == '\n' || c == '\r') {
                break;
            }
            App_push_entry_char(a, c);
        }
        tlk_cursor_mv(&(Pos){.c = a->size.c / 2 - 4, .r = 2});
        tlk_draw("NEW TASK", (Style){.fg = RED, .bold = true});

        tlk_cursor_mv(&(Pos){.c = a->size.c / 2 - (a->entry_buf_amt / 2), .r = 3});
        tlk_clear_current_line();
        if (a->entry_buf_amt > 0) {
            tlk_draw(a->entry_buf, tlk_style_default());
        } else {
            tlk_draw(" ", tlk_style_default());
        }
        fflush(stdout);
    }
    tlk_cursor_mv(&(Pos){.c = a->size.c / 2 - 4, .r = 2});
    tlk_clear_current_line();
    tlk_cursor_mv(&(Pos){.c = a->size.c / 2 - (a->entry_buf_amt / 2), .r = 3});
    tlk_clear_current_line();
    a->in_entry = false;
    if (cancel) {
        App_clear_entry_buff(a);
        return;
    }

    App_push_item(a, &(Item){.content = al_strdup(a->entry_buf), .s = NOT_DONE});
    App_clear_entry_buff(a);
}

void toggle_item_status(App *a) {
    for (size_t i = 0; i < a->items_amt; i++) {
        if ((int)i == a->item_sel) {
            a->items[i].s = !a->items[i].s;
        }
    }
}

#define ITEM_MV                                                                \
    tlk_cursor_mv(                                                               \
            &(Pos){.c = center_col - ((strlen(a->items[i].content) + 4) / 2),        \
            .r = initial_row + i});

void draw_items(App *a) {
    unsigned int center_col = a->size.c / 2;
    unsigned int initial_row = (a->size.r / 2) - (a->items_amt / 2);
    char *done;
    for (size_t i = 0; i < a->items_amt; i++) {
        if (a->items[i].s == DONE) {
            done = "[*] ";
        } else {
            done = "[ ] ";
        }
        if ((int)i == a->item_sel) {
            Style st = {.bg = WHITE, .fg = BLACK, .bold = true};
            for (unsigned int k = 0; k < a->size.c; k++) {
                tlk_cursor_mv(&(Pos){.c = k, .r = initial_row + i});
                tlk_draw(" ", st);
            }
            ITEM_MV;
            tlk_draw(done, st);
            tlk_draw(a->items[i].content, st);
            continue;
        }
        Style st = {.bg = DEFAULT, .fg = DEFAULT};
        ITEM_MV;
        tlk_clear_current_line();
        tlk_draw(done, st);
        tlk_draw(a->items[i].content, st);
    }
}

void draw_header(unsigned int *col) {
    for (unsigned int i = 0; i < *col; i++) {
        tlk_cursor_mv(&(Pos){.c = i, .r = 0});
        tlk_draw(" ", (Style){.bg = BLUE, .fg = BLACK, .bold = true});
    }
    tlk_cursor_mv(&(Pos){.c = ((*col / 2) - 2), .r = 0});
    tlk_draw("TODO", (Style){.bg = BLUE, .fg = BLACK, .bold = true});
}

void App_init(App *a) {
    a->size = tlk_terminal_size();
    a->should_quit = false;
    a->redraw = true;

    a->item_sel = 0;
    a->item_toggle = false;
    a->item_del = false;
    a->items = (Item *)malloc(8 * sizeof(Item));
    a->items_amt = 0;

    a->in_entry = false;
    a->entry_buf = (char *)malloc(8 * sizeof(char));
    a->entry_buf_amt = 0;
}

void App_clear_entry_buff(App *a) {
    char *tmp = realloc(a->entry_buf, 8 * sizeof(char));
    a->entry_buf = tmp;
    a->entry_buf_amt = 0;
    a->entry_buf[a->entry_buf_amt] = '\0';
}

void App_push_entry_char(App *a, char c) {
    char *tmp = realloc(a->entry_buf, (a->entry_buf_amt + 2) * sizeof(char));
    if (!tmp) {
        ERROR("Allocation failure");
        return;
    }
    a->entry_buf = tmp;
    a->entry_buf[a->entry_buf_amt] = c;
    a->entry_buf[a->entry_buf_amt + 1] = '\0';
    a->entry_buf_amt++;
}
void App_remove_last_entry_char(App *a) {
    if (a->entry_buf_amt == 0) {
        return;
    }
    a->entry_buf_amt--;
    a->entry_buf[a->entry_buf_amt] = '\0';
    char *tmp = realloc(a->entry_buf, (a->entry_buf_amt + 1) * sizeof(char));
    if (tmp || a->entry_buf_amt == 0) {
        a->entry_buf = tmp;
    }
}

void App_push_item(App *a, Item *i) {
    Item *tmp = realloc(a->items, (a->items_amt + 1) * sizeof(Item));
    if (!tmp) {
        ERROR("Allocation failure");
        return;
    }
    a->items = tmp;
    a->items[a->items_amt] = *i;
    a->items_amt++;
}

void controls(App *a) {
    a->k = tlk_key();
    if (a->k == ESCAPE || a->k == LOWER_Q || a->k == CTRL_C) {
        a->should_quit = true;
        return;
    }
    if (a->k == ARROW_DOWN) {
        a->item_sel++;
        if (a->item_sel == (int)a->items_amt) {
            a->item_sel = (int)a->items_amt;
        }
    }
    if (a->k == ARROW_UP) {
        a->item_sel--;
        if (a->item_sel < 0) {
            a->item_sel = 0;
        }
    }
    if (a->k == LOWER_T || a->k == UPPER_T) {
        a->item_toggle = true;
    }

    if (a->k == LOWER_N || a->k == UPPER_N) {
        a->in_entry = true;
    }
    if (a->k == LOWER_D || a->k == UPPER_D) {
        a->item_del = true;
    }
}

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

#endif
