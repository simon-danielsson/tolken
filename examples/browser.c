#ifdef BROWSER

#include <dirent.h>
#include <stdio.h>

#define TLK_IMPLEMENTATION
#include "../tlk.h"
#define ANA_IMPLEMENTATION
#include "../src/libs/ana.h"

typedef enum {
    IS_FILE,
    IS_DIR,
    IS_ELSE,
} PathType;

typedef struct {
    PathType type;
    float size_bytes;
    char *path;
    char *disp_name;
} Item;

typedef struct {
    Key k;
    Pos size;
    bool should_quit;
    bool should_open;
    bool hier_mv_dn;
    bool hier_mv_up;
    int item_sel;
    Item *items;
    size_t items_amt;
    char *current_dir;
} Browser;

void setup(void);
void cleanup(void);
void Browser_init(Browser *b);
void Browser_push_item(Browser *b, Item *i);
void Browser_gen_items(Browser *b);
void open_item(Browser *b);
void controls(Browser *b);
void draw_header(Browser *b);
void draw_items(Browser *b, unsigned int init_row);
void draw_current_dir(Browser *b, unsigned int row);
void hier_mv_dn(Browser *b);
void hier_mv_up(Browser *b);

int main() {
    setup();
    Browser b;
    Browser_init(&b);
    Pos term_size;
    Browser_gen_items(&b);

    while (!b.should_quit) {

        // update
        {
            term_size = tlk_terminal_size();
            if (term_size.c != b.size.c || term_size.r != b.size.r) {
                b.size = term_size;
            }
            controls(&b);

            if (b.should_open) {
                open_item(&b);
                b.should_open = false;
            }

            if (b.hier_mv_up) {
                hier_mv_up(&b);
                b.hier_mv_up = false;
            }

            if (b.hier_mv_dn) {
                hier_mv_dn(&b);
                b.hier_mv_dn = false;
            }
        }

        // draw
        {
            draw_header(&b);
            draw_current_dir(&b, 2);
            draw_items(&b, 4);
        }
        tlk_flush();
    }
    cleanup();
    return 0;
}

void open_item(Browser *b) {
    for (size_t i = 0; i < b->items_amt; i++) {
        if ((int)i == b->item_sel) {
#ifdef __APPLE__
            char *cmd = "open";
#elif defined(__linux__)
            char *cmd = "xdg-open";
#else
            ERROR("Unknown or non-unix platform detected\n");
#endif

            pid_t pid = fork();

            if (pid == 0) {
                execlp(cmd, cmd, b->items[i].path, (char *)NULL);
                _exit(127); // exec failed
            } else if (pid > 0) {
                cleanup();
                exit(0);
            } else {
                perror("fork");
            }

            break;
        }
    }
}

void Browser_reset_items(Browser *b) {
    for (size_t i = 0; i < b->items_amt; i++) {
        free(b->items[i].disp_name);
        free(b->items[i].path);
    }
    free(b->items);

    // new items
    b->items = (Item *)malloc(8 * sizeof(Item));
    b->items_amt = 0;
    b->item_sel = 0;
}

char *get_parent_dir(const char *f) {
    const char *last_slash = strrchr(f, '/');
    if (last_slash == NULL) {
        return al_strdup(".");
    }
    if (last_slash == f) {
        return al_strdup("/");
    }
    size_t len = (size_t)(last_slash - f);
    char *output = malloc(len + 1);
    if (!output) {
        return NULL;
    }
    memcpy(output, f, len);
    output[len] = '\0';
    return output;
}

void hier_mv_up(Browser *b) {
    // get new current_dir
    for (size_t i = 0; i < b->items_amt; i++) {
        if ((int)i == b->item_sel) {
            b->current_dir = get_parent_dir(b->current_dir);
            if (b->current_dir == NULL) {
                ERROR("parent dir is NULL");
            }
        }
    }

    Browser_reset_items(b);

    tlk_screen_clear();
    Browser_gen_items(b);
}

void hier_mv_dn(Browser *b) {
    // get new current_dir
    for (size_t i = 0; i < b->items_amt; i++) {
        if ((int)i == b->item_sel) {
            if (b->items[i].type == IS_DIR) {
                free(b->current_dir);
                b->current_dir = al_strdup(b->items[i].path);
                Browser_reset_items(b);
                tlk_screen_clear();
                Browser_gen_items(b);
                return;
            }
        }
    }
}

void draw_current_dir(Browser *b, unsigned int row) {
    tlk_cursor_mv(
            &(Pos){.c = b->size.c / 2 - (strlen(b->current_dir) / 2), .r = row});
    tlk_clear_current_line();
    tlk_draw(b->current_dir, tlk_style_default());
    for (unsigned int i = 0; i < b->size.c; i++) {
        tlk_cursor_mv(&(Pos){.c = i, .r = row + 1});
        tlk_draw("┈", tlk_style_default());
    }
}

void draw_items(Browser *b, unsigned int init_row) {
    char display[(b->size.c * 2) + 1];
    char size[128];
    Style st;
    char spacing[b->size.c + 1];
    int spacing_w;
    int icon_w = 2;
    char icon[10];

    for (unsigned int i = 0; i < b->items_amt; i++) {
        if ((int)i == b->item_sel) {
            st = (Style){.bg = MAGENTA, .fg = BLACK, .bold = true};
        } else {
            st = (Style){.bg = DEFAULT, .fg = DEFAULT, .bold = false};
        }

        tlk_cursor_mv(&(Pos){.c = 0, .r = init_row + i});
        tlk_clear_current_line();

        switch (b->items[i].type) {
            case IS_DIR:
                snprintf(icon, sizeof(icon), "󰉖 ");
                break;
            case IS_FILE:
                snprintf(icon, sizeof(icon), " ");
                break;
            default:
                snprintf(icon, sizeof(icon), " ");
        }

        snprintf(size, sizeof(size), " %.1fB", b->items[i].size_bytes);
        spacing_w =
            b->size.c - strlen(size) - icon_w - strlen(b->items[i].disp_name);
        if (spacing_w < 0)
            spacing_w = 0;
        for (int j = 0; j < spacing_w; j++) {
            spacing[j] = (j % 2 == 0) ? ' ' : '-';
        }
        spacing[spacing_w] = '\0';
        snprintf(display, sizeof(display), "%s%s%s%s", icon, b->items[i].disp_name,
                spacing, size);
        tlk_draw(display, st);
    }

    // clear overflow
    tlk_cursor_mv(&(Pos){.c = 0, .r = init_row + b->items_amt});
    tlk_clear_current_line();
}

char *get_current_dir(void) {
    char cwd[PATH_MAX];
    if (getcwd(cwd, sizeof(cwd)) != NULL) {
        return al_strdup(cwd);
    } else {
        ERROR("getcwd() error");
    }
}

void draw_header(Browser *b) {
    Style st = {.fg = BLACK, .bg = MAGENTA, .bold = false};
    Style stb = {.fg = BLACK, .bg = MAGENTA, .bold = true};
    for (unsigned int i = 0; i < b->size.c; i++) {
        tlk_cursor_mv(&(Pos){.c = i, .r = 0});
        tlk_draw(" ", st);
    }

    tlk_cursor_mv(&(Pos){.c = (b->size.c / 2) - (36 / 2), .r = 0});
    tlk_draw("q ", stb);
    tlk_draw("quit • ", st);
    tlk_draw("o ", stb);
    tlk_draw("open • ", st);
    tlk_draw("↓/↑ ", stb);
    tlk_draw("nav • ", st);
    tlk_draw("←/→ ", stb);
    tlk_draw("move", st);
}

void Browser_push_item(Browser *b, Item *i) {
    Item *tmp = realloc(b->items, (b->items_amt + 1) * sizeof(Item));
    if (!tmp) {
        ERROR("Allocation failure");
        return;
    }
    b->items = tmp;
    b->items[b->items_amt] = *i;
    b->items_amt++;
}

PathType check_path_type(const char *path) {
    struct stat path_stat;

    if (lstat(path, &path_stat) != 0) {
        ERROR("Path does not exist: %s", path);
    }

    if (S_ISLNK(path_stat.st_mode)) {
        return IS_ELSE;
    }

    if (S_ISREG(path_stat.st_mode)) {
        return IS_FILE;
    } else if (S_ISDIR(path_stat.st_mode)) {
        return IS_DIR;
    } else {
        return IS_ELSE;
    }
}

void Browser_gen_items(Browser *b) {
    struct dirent *file_info;
    DIR *dir = opendir(b->current_dir);
    if (dir == NULL) {
        return;
    }

    while ((file_info = readdir(dir)) != NULL) {
        if (strcmp(file_info->d_name, ".") == 0 ||
                strcmp(file_info->d_name, "..") == 0) {
            continue;
        }

        char file_path[1000];
        snprintf(file_path, sizeof(file_path), "%s/%s", b->current_dir,
                file_info->d_name);

        struct stat st;
        size_t size = 0;

        if (stat(file_path, &st) == 0) {
            size = st.st_size;
        }

        Browser_push_item(b, &(Item){.size_bytes = size,
                .disp_name = al_strdup(file_info->d_name),
                .type = check_path_type(file_path),
                .path = al_strdup(file_path)});
    }

    closedir(dir);
}

void Browser_init(Browser *b) {
    b->should_quit = false;
    b->should_open = false;
    b->hier_mv_dn = false;
    b->hier_mv_up = false;

    b->size = tlk_terminal_size();

    b->current_dir = get_current_dir();

    b->items = (Item *)malloc(8 * sizeof(Item));
    b->items_amt = 0;
    b->item_sel = 0;
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

void controls(Browser *b) {
    b->k = tlk_key();
    if (b->k == ESCAPE || b->k == LOWER_Q || b->k == CTRL_C) {
        b->should_quit = true;
        return;
    }

    if (b->k == LOWER_O || b->k == UPPER_O) {
        b->should_open = true;
    }

    if (b->k == ARROW_LEFT) {
        b->hier_mv_up = true;
    }
    if (b->k == ARROW_RIGHT) {
        b->hier_mv_dn = true;
    }
    if (b->k == ARROW_DOWN) {
        b->item_sel++;
        if (b->item_sel >= (int)b->items_amt) {
            b->item_sel = (int)b->items_amt - 1;
        }
    }
    if (b->k == ARROW_UP) {
        b->item_sel--;
        if (b->item_sel < 0) {
            b->item_sel = 0;
        }
    }
}

#endif
