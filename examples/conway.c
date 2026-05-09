//! example conway's game of life
#ifdef CONWAY

#include <stdio.h>
#include <time.h>

#define TLK_IMPLEMENTATION
#include "../tlk.h"
#define ANA_IMPLEMENTATION
#include "../src/libs/ana.h"

typedef struct {
    Pos p;
    bool alive;
    bool prev_state;
    int live_neighbors;
} Cell;

typedef struct {
    Pos size;
    bool should_quit;
    Cell *cells;
    unsigned int cell_count;
} GameOfLife;

void setup(void);
void cleanup(void);
void GameOfLife_init(GameOfLife *gol);
void controls(GameOfLife *gol);
void update_cells(GameOfLife *gol);

int main() {
    setup();
    GameOfLife gol;
    GameOfLife_init(&gol);
    int generations;
    while (!gol.should_quit) {
        GameOfLife_init(&gol);
        generations = 0;
        while (generations < 100 && !gol.should_quit) {
            controls(&gol);
            // update
            update_cells(&gol);
            // draw
            for (unsigned int i = 0; i < gol.cell_count; i++) {
                tlk_cursor_mv(&gol.cells[i].p);
                if (!gol.cells[i].alive &&
                        gol.cells[i].alive != gol.cells[i].prev_state) {
                    tlk_draw(".", tlk_style_default());
                    continue;
                }
                if (gol.cells[i].alive &&
                        gol.cells[i].alive != gol.cells[i].prev_state) {
                    tlk_draw("█", tlk_style_default());
                }
            }

            char buff[100];
            snprintf(buff, sizeof(buff), "Generation: %d", generations);
            tlk_cursor_move_home();
            tlk_draw(buff, tlk_style_default());

            // cycle
            tlk_flush();
            generations++;
        }
    }
    cleanup();
    return 0;
}

void GameOfLife_init(GameOfLife *gol) {
    gol->size = tlk_terminal_size();
    gol->should_quit = false;
    gol->cells = (Cell *)malloc(sizeof(Cell) * (gol->size.c * gol->size.r));
    int counter = 0;
    for (unsigned int i = 0; i < gol->size.r; i++) {
        for (unsigned int j = 0; j < gol->size.c; j++) {
            gol->cells[counter].alive = rand() % 2 ? true : false;
            gol->cells[counter].p = (Pos){.c = j, .r = i};
            gol->cells[counter].prev_state = gol->cells[counter].alive;
            counter++;
        }
    }
    gol->cell_count = counter;
}

void update_cells(GameOfLife *gol) {
    bool next_alive[gol->cell_count];
    Pos neighbor;
    int live_neighbors;

    for (unsigned int k = 0; k < gol->cell_count; k++) {
        live_neighbors = 0;
        for (int i = -1; i <= 1; i++) {
            for (int j = -1; j <= 1; j++) {
                if (i == 0 && j == 0) {
                    continue;
                }

                neighbor =
                    (Pos){.r = gol->cells[k].p.r + i, .c = gol->cells[k].p.c + j};

                for (unsigned int p = 0; p < gol->cell_count; p++) {
                    if (gol->cells[p].p.r == neighbor.r &&
                            gol->cells[p].p.c == neighbor.c && gol->cells[p].alive) {
                        live_neighbors++;
                    }
                }
            }
        }

        if (gol->cells[k].alive) {
            next_alive[k] = live_neighbors == 2 || live_neighbors == 3;
        } else {
            next_alive[k] = live_neighbors == 3;
        }
        gol->cells[k].live_neighbors = live_neighbors;
    }

    for (unsigned int k = 0; k < gol->cell_count; k++) {
        gol->cells[k].prev_state = gol->cells[k].alive;
        gol->cells[k].alive = next_alive[k];
    }
}

void setup(void) {
    tlk_cursor_save_pos();
    tlk_screen_save();
    tlk_cursor_hide();
    tlk_enable_raw_mode();
    tlk_screen_clear();
    srandom(time(NULL));
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

void controls(GameOfLife *gol) {
    Key key = tlk_key();
    if (key == ESCAPE || key == Q || key == CTRL_C) {
        gol->should_quit = true;
        return;
    }
}

#endif
