/* ==========================================================================
 * input.c — Debounced button reading with direction locking
 * EE3463 Final Project — 3D Snek :3
 *
 * Timer0 fires every ~10 ms.  Each interrupt samples all four buttons.
 * A press is only accepted after DEBOUNCE_COUNT consecutive identical
 * reads (3 × 10 ms = 30 ms).  Direction locking prevents the player
 * from reversing the snake directly into itself.
 * ========================================================================== */

#include <avr/io.h>
#include <avr/interrupt.h>
#include "config.h"
#include "input.h"

/* ---------- internal state ---------------------------------------------- */

/* The direction that has passed debounce validation. */
static volatile Direction validated_dir = DIR_RIGHT;

/* The direction the snake is actually heading (set by game_update).
 * Used to enforce the no-180° rule.                                       */
static volatile Direction current_snake_dir = DIR_RIGHT;

/* Debounce counters — one per button. */
static volatile uint8_t debounce_up    = 0;
static volatile uint8_t debounce_down  = 0;
static volatile uint8_t debounce_left  = 0;
static volatile uint8_t debounce_right = 0;

/* ==========================================================================
 * input_init — configure button pins as inputs with pull-ups
 * ========================================================================== */
void input_init(void)
{
    /* Set PD2..PD5 as inputs (clear DDR bits). */
    BTN_DDR &= ~BTN_ALL_MASK;

    /* Enable internal pull-ups (write 1 to PORT while DDR = 0). */
    BTN_PORT |= BTN_ALL_MASK;
}

/* ==========================================================================
 * input_get_direction — return the latest debounced, lock-validated dir
 * ========================================================================== */
Direction input_get_direction(void)
{
    Direction d;
    cli();
    d = validated_dir;
    sei();
    return d;
}

/* ==========================================================================
 * input_set_current_direction — called by game.c so we know what to lock
 * ========================================================================== */
void input_set_current_direction(Direction d)
{
    current_snake_dir = d;
}

/* ==========================================================================
 * opposite — helper: returns 1 if a is the reverse of b
 * ========================================================================== */
static uint8_t opposite(Direction a, Direction b)
{
    /* UP↔DOWN and LEFT↔RIGHT differ by exactly 1 when paired as 0,1 / 2,3 */
    if (a == DIR_UP    && b == DIR_DOWN)  return 1;
    if (a == DIR_DOWN  && b == DIR_UP)    return 1;
    if (a == DIR_LEFT  && b == DIR_RIGHT) return 1;
    if (a == DIR_RIGHT && b == DIR_LEFT)  return 1;
    return 0;
}

/* ==========================================================================
 * try_accept — if debounce counter hits threshold, accept the direction
 *              (unless it's a 180° reversal).
 * ========================================================================== */
static void try_accept(Direction candidate, volatile uint8_t *counter, uint8_t pressed)
{
    if (pressed) {
        if (*counter < DEBOUNCE_COUNT) {
            (*counter)++;
        }
        if (*counter >= DEBOUNCE_COUNT) {
            /* Only accept if it doesn't reverse into the snake. */
            if (!opposite(candidate, current_snake_dir)) {
                validated_dir = candidate;
            }
        }
    } else {
        *counter = 0;          /* button released — reset counter */
    }
}

/* ==========================================================================
 * Timer0 compare-match ISR — runs every ~10 ms, samples all buttons
 * ========================================================================== */
ISR(TIMER0_COMPA_vect)
{
    uint8_t pins = BTN_PIN;    /* read all of PORTD at once for consistency */

    /* Active-low: pressed when bit is 0. */
    try_accept(DIR_UP,    &debounce_up,    !(pins & BTN_UP_MASK));
    try_accept(DIR_DOWN,  &debounce_down,  !(pins & BTN_DOWN_MASK));
    try_accept(DIR_LEFT,  &debounce_left,  !(pins & BTN_LEFT_MASK));
    try_accept(DIR_RIGHT, &debounce_right, !(pins & BTN_RIGHT_MASK));
}
