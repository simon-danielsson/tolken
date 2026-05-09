//! example snake game
#ifdef SNAKE

#include <stdio.h>
#include <time.h>

#define TLK_IMPLEMENTATION
#include "../tlk.h"
#define ANA_IMPLEMENTATION
#include "../src/libs/ana.h"

typedef enum { UP, DOWN, LEFT, RIGHT } Dir;

typedef struct {
    Pos current;
    Pos prev;
} TailSegment;

typedef struct {
    // player
    Pos head;
    TailSegment *tail;
    size_t tail_size;
    Dir dir;
    // general
    Pos food;
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
bool food_collision(Snake *sn);
bool boundary_collision(Snake *sn);
bool snake_collision(Snake *sn);
void extend_tail(Snake *sn, TailSegment *new_tail);
void update_snake_pos(Snake *sn, Pos *head_prev_pos);
void game_over(int score);

#define SCORE_COL WHITE
#define FOOD_COL RED
#define FOOD_ICO "O"
#define SNAKE_COL MAGENTA
#define SNAKE_ICO "█"

int main(void) {
    setup();
    Snake sn;
    Snake_init(&sn);

    char score_buf[32];
    Pos head_prev_pos;

    // init food
    gen_new_food(&sn);

    while (!sn.should_quit) {
        // update --------------------------------------------------------------
        head_prev_pos = sn.head;
        // term size
        sn.size = tlk_terminal_size();

        update_snake_pos(&sn, &head_prev_pos);

        // collisions ------------------------------------------------------

        // TODO: issue with food collision. snake does not collide with food
        // sometimes when the food is right on the border of the game. (happens
        // most of the time when the food is to the left border)

        if (boundary_collision(&sn) || snake_collision(&sn)) {
            game_over(sn.score);
            sn.should_quit = true;
            break;
        }

        if (food_collision(&sn)) {
            if (sn.tail_size == 0) {
                extend_tail(&sn, &(TailSegment){.current = head_prev_pos,
                        .prev = head_prev_pos});
            } else {
                extend_tail(&sn,
                        &(TailSegment){.current = sn.tail[sn.tail_size - 1].prev,
                        .prev = sn.tail[sn.tail_size - 1].prev});
            }

            tlk_cursor_mv(&(Pos){.c = sn.food.c, .r = sn.food.r});
            tlk_clear_at_cursor_pos();
            gen_new_food(&sn);
        }

        // draw ----------------------------------------------------------------
        {
            // score
            if (sn.new_score) {
                score_buf[0] = '\0';
                snprintf(score_buf, sizeof(score_buf), " Score: %d ", sn.score);
                for (unsigned int i = 0; i < sn.size.c; i++) {
                    tlk_cursor_mv(&(Pos){.c = i, .r = 0});
                    tlk_draw(" ", (Style){.fg = BLACK, .bg = SCORE_COL});
                }
                tlk_cursor_mv(
                        &(Pos){.c = sn.size.c / 2 - (strlen(score_buf) / 2), .r = 0});
                tlk_draw(score_buf,
                        (Style){.fg = BLACK, .bg = SCORE_COL, .bold = true});
                sn.new_score = false;
            }

            // snake
            tlk_cursor_mv(&head_prev_pos);
            tlk_clear_at_cursor_pos();
            tlk_cursor_mv(&(Pos){.c = sn.head.c, .r = sn.head.r});
            tlk_draw(SNAKE_ICO, (Style){.fg = SNAKE_COL, .bg = DEFAULT});

            for (size_t i = 0; i < sn.tail_size; i++) {
                tlk_cursor_mv(&sn.tail[i].prev);
                tlk_clear_at_cursor_pos();
                sn.tail[i].prev = sn.tail[i].current;
                tlk_cursor_mv(
                        &(Pos){.c = sn.tail[i].current.c, .r = sn.tail[i].current.r});
                tlk_draw(SNAKE_ICO, (Style){.fg = SNAKE_COL, .bg = DEFAULT});
            }

            // food
            tlk_cursor_mv(&(Pos){.c = sn.food.c, .r = sn.food.r});
            tlk_draw(FOOD_ICO, (Style){.fg = FOOD_COL, .bg = DEFAULT});
        }

        // after ---------------------------------------------------------------
        al_sleepf(0.2);
        tlk_flush();
        controls(&sn);
    }
    cleanup();

    return 0;
}

bool boundary_collision(Snake *sn) {
    if (sn->head.c > sn->size.c || sn->head.c < 0 || sn->head.r > sn->size.r ||
            sn->head.r < 1) {
        return true;
    }
    for (size_t i = 0; i < sn->tail_size; i++) {
        if (sn->tail[i].current.c > sn->size.c || sn->tail[i].current.c < 0 ||
                sn->tail[i].current.r > sn->size.r || sn->tail[i].current.r < 1) {
            return true;
        }
    }
    return false;
}

bool snake_collision(Snake *sn) {
    if (sn->tail_size <= 1) {
        return false;
    }
    for (size_t i = 1; i < sn->tail_size - 1; i++) {
        if (sn->tail[i].current.c == sn->head.c &&
                sn->tail[i].current.r == sn->head.r) {
            return true;
        }
    }
    return false;
}

void game_over(int score) {
    tlk_screen_clear();
    Pos size = tlk_terminal_size();
    char buff[256];
    snprintf(buff, sizeof(buff), " Total score: %d ", score);
    tlk_cursor_mv(&(Pos){.c = size.c / 2 - 5, .r = size.r / 2 - 1});
    tlk_draw("You lost!", tlk_style_default());
    tlk_cursor_mv(
            &(Pos){.c = size.c / 2 - (strlen(buff) / 2), .r = size.r / 2 + 1});
    tlk_draw(buff, tlk_style_default());
    tlk_flush();
    al_sleep(5);
}

#define UPDATE_TAIL                                                            \
    for (size_t i = 0; i < sn->tail_size; i++) {                                 \
        if (i == 0) {                                                              \
            sn->tail[i].current = *head_prev_pos;                                    \
        } else {                                                                   \
            sn->tail[i].current = sn->tail[i - 1].prev;                              \
        }                                                                          \
    }

void update_snake_pos(Snake *sn, Pos *head_prev_pos) {
    switch (sn->dir) {
        case LEFT:
            sn->head.c--;
            UPDATE_TAIL
                break;

        case RIGHT:
            sn->head.c++;
            UPDATE_TAIL
                break;

        case UP:
            sn->head.r--;
            UPDATE_TAIL
                break;

        case DOWN:
            sn->head.r++;
            UPDATE_TAIL
                break;
    }
}

void extend_tail(Snake *sn, TailSegment *new_tail) {
    TailSegment *tmp =
        realloc(sn->tail, (sn->tail_size + 1) * sizeof(TailSegment));
    sn->tail = tmp;
    sn->tail[sn->tail_size] = *new_tail;
    sn->tail_size++;
}

bool food_collision(Snake *sn) {
    if (sn->head.c == sn->food.c && sn->head.r == sn->food.r) {
        sn->score += 5;
        sn->new_score = true;
        return true;
    }
    for (size_t i = 0; i < sn->tail_size; i++) {
        if (sn->tail[i].current.c == sn->food.c &&
                sn->tail[i].current.r == sn->food.r) {
            sn->score += 5;
            sn->new_score = true;
            return true;
        }
    }

    return false;
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
    sn->food =
        (Pos){.c = random() % sn->size.c, .r = 1 + (random() % (sn->size.r - 1))};
}

void Snake_init(Snake *sn) {
    Pos size = tlk_terminal_size();
    sn->head = (Pos){.c = size.c / 2, .r = size.r / 2};
    sn->tail = NULL;
    sn->tail_size = 0;
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

#endif
