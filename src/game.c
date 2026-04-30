/* ==========================================================================
 * game.c — Snake game logic
 * EE3463 Final Project — 3D Snek :3
 *
 * The snake body is stored in a circular buffer of Point structs.
 * head and tail indices chase each other around the buffer.
 *
 * Layer 0 (bottom): eat 5 treats to win, tick = 400 ms
 * Layer 1 (top):    eat 8 treats to win, tick = 250 ms
 * Wall behaviour:   wrap-around (toroidal grid)
 * Self-collision:   game over
 * ========================================================================== */

#include <stdlib.h>
#include <avr/io.h>
#include "config.h"
#include "game.h"
#include "input.h"
#include "timer.h"
#include "display.h"

/* ---------- internal state ---------------------------------------------- */

/* Circular buffer for the snake body. */
static Point body[MAX_SNAKE_LENGTH];
static uint8_t head_idx;          /* index of the head segment              */
static uint8_t tail_idx;          /* index of the tail segment              */
static uint8_t snake_length;

/* Current treat position. */
static Point treat;

/* Current movement direction (applied each tick). */
static Direction dir;

/* Which layer we're playing (0 or 1). */
static uint8_t current_layer;

/* How many treats eaten on the current layer. */
static uint8_t treats_eaten;

/* High-level game state. */
static GameState state;

/* ---------- forward declarations ---------------------------------------- */
static void place_treat(void);
static uint8_t is_on_snake(uint8_t x, uint8_t y);
static void    start_layer(uint8_t layer);
static uint16_t read_adc_for_seed(void);

/* ==========================================================================
 * read_adc_for_seed — read the ADC once from a floating pin (PC0) to get
 *                     an unpredictable-ish value for srand().
 * ========================================================================== */
static uint16_t read_adc_for_seed(void)
{
    /* AVcc reference, right-adjusted, channel 0 (PC0). */
    ADMUX  = (1 << REFS0) | (RNG_ADC_CHANNEL & 0x0F);

    /* Enable ADC, single conversion, prescaler 128 (125 kHz ADC clock). */
    ADCSRA = (1 << ADEN) | (1 << ADSC)
           | (1 << ADPS2) | (1 << ADPS1) | (1 << ADPS0);

    /* Wait for conversion to finish. */
    while (ADCSRA & (1 << ADSC))
        ;

    uint16_t val = ADC;      /* read full 10-bit result */

    /* Disable ADC to save power (not needed during gameplay). */
    ADCSRA = 0;

    return val;
}

/* ==========================================================================
 * start_layer — reset the snake and treat for a given layer
 * ========================================================================== */
static void start_layer(uint8_t layer)
{
    current_layer = layer;
    treats_eaten  = 0;

    /* Initial snake: 3 segments, centre-left, heading RIGHT.
     * Tail at (1,3), body (2,3), head (3,3).                              */
    snake_length = INITIAL_SNAKE_LEN;
    tail_idx     = 0;
    head_idx     = INITIAL_SNAKE_LEN - 1;

    for (uint8_t i = 0; i < INITIAL_SNAKE_LEN; i++) {
        body[i].x = (uint8_t)(1 + i);   /* columns 1, 2, 3 */
        body[i].y = 3;                   /* row 3 (middle-ish) */
    }

    dir = DIR_RIGHT;
    input_set_current_direction(DIR_RIGHT);

    /* Set game tick speed for this layer. */
    timer_set_tick_period(layer == 0 ? TICK_SLOW : TICK_FAST);

    /* Set the physical layer select pin. */
    display_set_layer(layer);

    /* Place the first treat. */
    place_treat();

    state = STATE_PLAYING;
}

/* ==========================================================================
 * game_init — first-time initialisation, called once from main()
 * ========================================================================== */
void game_init(void)
{
    /* Seed the PRNG from a noisy ADC reading. */
    srand(read_adc_for_seed());

    start_layer(0);
}

/* ==========================================================================
 * place_treat — put a treat on a random empty cell
 * ========================================================================== */
static void place_treat(void)
{
    uint8_t x, y;
    do {
        x = (uint8_t)(rand() & 0x07);   /* 0–7 */
        y = (uint8_t)(rand() & 0x07);
    } while (is_on_snake(x, y));

    treat.x = x;
    treat.y = y;
}

/* ==========================================================================
 * is_on_snake — return 1 if (x,y) overlaps any snake segment
 * ========================================================================== */
static uint8_t is_on_snake(uint8_t x, uint8_t y)
{
    for (uint8_t i = 0; i < snake_length; i++) {
        uint8_t idx = (tail_idx + i) & (MAX_SNAKE_LENGTH - 1);
        if (body[idx].x == x && body[idx].y == y) {
            return 1;
        }
    }
    return 0;
}

/* ==========================================================================
 * game_update — advance the game by one tick
 *
 * Called from the main loop whenever game_tick_flag is set.
 * ========================================================================== */
void game_update(void)
{
    if (state != STATE_PLAYING) {
        return;                 /* animations handled elsewhere */
    }

    /* ---- 1. Read the current input direction ---- */
    dir = input_get_direction();
    input_set_current_direction(dir);

    /* ---- 2. Compute the new head position (wrap-around) ---- */
    Point new_head;
    Point old_head = body[head_idx];

    switch (dir) {
        case DIR_UP:
            new_head.x = old_head.x;
            new_head.y = (old_head.y == 0) ? (GRID_SIZE - 1) : (old_head.y - 1);
            break;
        case DIR_DOWN:
            new_head.x = old_head.x;
            new_head.y = (old_head.y + 1) & (GRID_SIZE - 1);
            break;
        case DIR_LEFT:
            new_head.x = (old_head.x == 0) ? (GRID_SIZE - 1) : (old_head.x - 1);
            new_head.y = old_head.y;
            break;
        case DIR_RIGHT:
        default:
            new_head.x = (old_head.x + 1) & (GRID_SIZE - 1);
            new_head.y = old_head.y;
            break;
    }

    /* ---- 3. Check for self-collision ---- */
    /* We check against the body BEFORE moving the tail forward.  If the
     * snake isn't growing, the tail cell will be vacated this tick, so we
     * skip the very last segment (the current tail).                       */
    uint8_t ate_treat = (new_head.x == treat.x && new_head.y == treat.y);

    for (uint8_t i = 0; i < snake_length; i++) {
        uint8_t idx = (tail_idx + i) & (MAX_SNAKE_LENGTH - 1);

        /* If NOT eating a treat, the tail is about to move forward, so
         * we can ignore the current tail position.                         */
        if (!ate_treat && i == 0) {
            continue;
        }

        if (body[idx].x == new_head.x && body[idx].y == new_head.y) {
            state = STATE_GAME_OVER;
            return;
        }
    }

    /* ---- 4. Place new head ---- */
    head_idx = (head_idx + 1) & (MAX_SNAKE_LENGTH - 1);
    body[head_idx] = new_head;

    if (ate_treat) {
        /* Grow: don't advance the tail. */
        snake_length++;
        treats_eaten++;

        /* Check win condition for this layer. */
        uint8_t target = (current_layer == 0) ? LAYER1_TREATS : LAYER2_TREATS;
        if (treats_eaten >= target) {
            if (current_layer == 0) {
                state = STATE_WIN_SEQUENCE;
            } else {
                state = STATE_VICTORY;
            }
            return;
        }

        /* Place a new treat. */
        place_treat();
    } else {
        /* No growth: advance the tail. */
        tail_idx = (tail_idx + 1) & (MAX_SNAKE_LENGTH - 1);
    }
}

/* ---------- accessors for display.c and main.c -------------------------- */

GameState game_get_state(void)
{
    return state;
}

uint8_t game_get_layer(void)
{
    return current_layer;
}

uint8_t game_get_treats_eaten(void)
{
    return treats_eaten;
}

const Point *game_get_body(void)
{
    return body;
}

uint8_t game_get_length(void)
{
    return snake_length;
}

uint8_t game_get_tail_index(void)
{
    return tail_idx;
}

Point game_get_treat(void)
{
    return treat;
}

/* ==========================================================================
 * Functions called by main.c after an animation completes to advance the
 * state machine.  We expose them via game.h would be cleaner, but to keep
 * the header minimal we just make them non-static here and declare extern
 * in main.c.
 * ========================================================================== */

void game_transition_to_layer2(void)
{
    start_layer(1);
}

void game_restart(void)
{
    srand(read_adc_for_seed());   /* re-seed for variety */
    start_layer(0);
}
