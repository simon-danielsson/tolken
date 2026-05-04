//! example: snake

#define TLK_IMPLEMENTATION
#define ANA_IMPLEMENTATION
#include "../src/libs/ana.h"
#include "../tlk.h"
#include <stdio.h>
#include <time.h>

typedef enum { UP, DOWN, LEFT, RIGHT } Dir;

typedef struct {
    // player
    Pos head;
    Pos *tail;
    size_t tail_size;
    Dir dir;
    // general
    Pos food;
    bool no_food;
    bool new_score;
    int score;
    Pos size;
    bool should_quit;
} Snake;

void setup(void);
void cleanup(void);
void Snake_init(Snake *sn);
void controls(Snake *sn);
void gen_new_food(Snake *sn);

#define SCORE_COL YELLOW
#define FOOD_COL RED
#define FOOD_ICO "⧳"
#define SNAKE_COL GREEN
#define SNAKE_ICO "█"

int main(void) {

    setup();
    Snake sn;
    Snake_init(&sn);

    char score_buf[32];
    Pos head_prev_pos;

    // tlk_screen_color(GREEN);
    while (!sn.should_quit) {
        // update --------------------------------------------------------------
        head_prev_pos = sn.head;
        {
            // snake
            switch (sn.dir) {
                case LEFT:
                    sn.head.c--;
                    break;
                case RIGHT:
                    sn.head.c++;
                    break;
                case UP:
                    sn.head.r--;
                    break;
                case DOWN:
                    sn.head.r++;
                    break;
            }

            // term size
            sn.size = tlk_terminal_size();

            // food
            if (sn.no_food) {
                tlk_cursor_mv(&(Pos){.c = sn.food.c, .r = sn.food.r});
                tlk_clear_at_cursor_pos();
                gen_new_food(&sn);
                sn.no_food = false;
            }
        }

        // draw ----------------------------------------------------------------
        {
            // score
            if (sn.new_score) {
                tlk_cursor_mv(&(Pos){.c = 0, .r = 0});
                score_buf[0] = '\0';
                snprintf(score_buf, sizeof(score_buf), " Score: %d ", sn.score);
                tlk_draw(score_buf, (Style){.fg = BLACK, .bg = SCORE_COL});
                sn.new_score = false;
            }

            // snake
            tlk_cursor_mv(&head_prev_pos);
            tlk_clear_at_cursor_pos();
            tlk_cursor_mv(&(Pos){.c = sn.head.c, .r = sn.head.r});
            tlk_draw(SNAKE_ICO, (Style){.fg = SNAKE_COL, .bg = DEFAULT});

            // food
            tlk_cursor_mv(&(Pos){.c = sn.food.c, .r = sn.food.r});
            tlk_draw(FOOD_ICO, (Style){.fg = FOOD_COL, .bg = DEFAULT});
            sn.no_food = true;
        }

        // after ---------------------------------------------------------------
        al_sleepf(0.1);
        tlk_flush();
        controls(&sn);
    }
    cleanup();

    return 0;
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

void gen_new_food(Snake *sn) {
    sn->food = (Pos){.c = random() % sn->size.c, .r = random() % sn->size.r};
}

void Snake_init(Snake *sn) {
    Pos size = tlk_terminal_size();
    sn->head = (Pos){.c = size.c / 2, .r = size.r / 2};
    sn->tail = NULL;
    sn->tail_size = 0;
    sn->no_food = true;
    sn->new_score = true;
    sn->dir = UP;
    sn->score = 0;
    sn->size = size;
    sn->should_quit = false;
}

void controls(Snake *sn) {
    Key key = tlk_key();

    if (key == ESCAPE || key == Q || key == CTRL_C) {
        sn->should_quit = true;
        return;
    }
    if (key == ARROW_LEFT) {
        sn->dir = LEFT;
        return;
    }
    if (key == ARROW_RIGHT) {
        sn->dir = RIGHT;
        return;
    }
    if (key == ARROW_UP) {
        sn->dir = UP;
        return;
    }
    if (key == ARROW_DOWN) {
        sn->dir = DOWN;
        return;
    }
}
