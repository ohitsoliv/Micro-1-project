/* ==========================================================================
 * display.c — 8×8 bicolor LED matrix driver
 * EE3463 Final Project — 3D Snek :3
 *
 * The display driver is hardware-agnostic.  It maintains two 8-byte frame
 * buffers (green_buffer, red_buffer) and fires a user-provided callback
 * from the Timer2 ISR (~1 kHz) with the row-select and column data for
 * each row.  Register your output function with display_set_output_fn()
 * before calling display_init().
 *
 * Callback parameters (called twice per row, every ~1 ms base tick):
 *   row_select  — active-low byte; bit N = 0 means row N is active
 *   green_cols  — column bits to light green for this row
 *   red_cols    — column bits to light red for this row
 *
 * Snake body → green,  Treat → red,  Overlap → both (yellow/amber).
 * ========================================================================== */

#include <avr/io.h>
#include <avr/interrupt.h>
#include <string.h>
#include <util/delay.h>
#include "config.h"
#include "display.h"
#include "game.h"

/* ---------- frame buffers (volatile — written by main, read by ISR) ----- */
volatile uint8_t green_buffer[8];
volatile uint8_t red_buffer[8];

/* ---------- current mux row (internal to ISR) --------------------------- */
static volatile uint8_t current_row = 0;

/* ---------- user-provided hardware output callback ---------------------- */
static display_output_fn output_fn = 0;

void display_set_output_fn(display_output_fn fn)
{
    output_fn = fn;
}

/* ==========================================================================
 * display_init — set up shift-register and layer-select pins
 * ========================================================================== */
void display_init(void)
{
    memset((void *)green_buffer, 0, 8);
    memset((void *)red_buffer,   0, 8);
    current_row = 0;
}

/* ==========================================================================
 * display_render — convert game state into green/red frame buffers
 *
 * Called once per game tick from the main loop.  The Timer2 ISR
 * continuously reads these buffers for multiplexing.
 * ========================================================================== */
void display_render(void)
{
    uint8_t g[8] = {0};
    uint8_t r[8] = {0};

    /* ---- draw snake body in green ---- */
    const Point *body   = game_get_body();
    uint8_t      len    = game_get_length();
    uint8_t      tail_i = game_get_tail_index();

    for (uint8_t i = 0; i < len; i++) {
        uint8_t idx = (tail_i + i) & (MAX_SNAKE_LENGTH - 1);
        g[body[idx].y] |= (1 << body[idx].x);
    }

    /* ---- draw treat in red ---- */
    Point treat = game_get_treat();
    r[treat.y] |= (1 << treat.x);

    /* ---- copy to volatile buffers atomically ---- */
    cli();
    memcpy((void *)green_buffer, g, 8);
    memcpy((void *)red_buffer,   r, 8);
    sei();
}

/* ==========================================================================
 * set_all — helper: fill both buffers with the same byte value
 * ========================================================================== */
static void set_all(uint8_t green_val, uint8_t red_val)
{
    cli();
    memset((void *)green_buffer, green_val, 8);
    memset((void *)red_buffer,   red_val,   8);
    sei();
}

/* ==========================================================================
 * display_win_animation — flash all LEDs yellow (green+red) 3 times
 * ========================================================================== */
void display_win_animation(void)
{
    for (uint8_t flash = 0; flash < 3; flash++) {
        set_all(0xFF, 0xFF);          /* all on (yellow) */
        _delay_ms(300);
        set_all(0x00, 0x00);          /* all off         */
        _delay_ms(300);
    }
}

/* ==========================================================================
 * display_game_over_animation — flash all LEDs red 3 times
 * ========================================================================== */
void display_game_over_animation(void)
{
    for (uint8_t flash = 0; flash < 3; flash++) {
        set_all(0x00, 0xFF);          /* red only */
        _delay_ms(300);
        set_all(0x00, 0x00);          /* off      */
        _delay_ms(300);
    }
}

/* ==========================================================================
 * display_victory_animation — spiral fill using both colours
 *
 * Fills the grid from the outside ring inward, alternating green and red
 * each ring, then clears from inside out.  Looks like a pulsing spiral.
 * ========================================================================== */
void display_victory_animation(void)
{
    /* We'll light up the border ring by ring. */
    /* Ring 0: rows 0,7 and cols 0,7
     * Ring 1: rows 1,6 and cols 1,6   ...up to ring 3 (centre 2×2)       */

    for (uint8_t repeat = 0; repeat < 2; repeat++) {
        uint8_t g[8] = {0};
        uint8_t r[8] = {0};

        /* Fill inward */
        for (uint8_t ring = 0; ring < 4; ring++) {
            uint8_t lo = ring;
            uint8_t hi = 7 - ring;
            uint8_t col_mask = 0;

            /* Build column mask for this ring */
            for (uint8_t c = lo; c <= hi; c++) {
                col_mask |= (1 << c);
            }

            /* Top and bottom rows of the ring */
            if (ring & 1) {
                /* odd rings → red */
                r[lo] |= col_mask;
                r[hi] |= col_mask;
            } else {
                /* even rings → green */
                g[lo] |= col_mask;
                g[hi] |= col_mask;
            }

            /* Left and right columns of inner rows */
            for (uint8_t row = lo + 1; row < hi; row++) {
                if (ring & 1) {
                    r[row] |= (1 << lo) | (1 << hi);
                } else {
                    g[row] |= (1 << lo) | (1 << hi);
                }
            }

            cli();
            memcpy((void *)green_buffer, g, 8);
            memcpy((void *)red_buffer,   r, 8);
            sei();
            _delay_ms(200);
        }

        _delay_ms(400);

        /* Clear outward */
        for (int8_t ring = 3; ring >= 0; ring--) {
            uint8_t lo = (uint8_t)ring;
            uint8_t hi = 7 - lo;
            uint8_t col_mask = 0;

            for (uint8_t c = lo; c <= hi; c++) {
                col_mask |= (1 << c);
            }

            g[lo] &= ~col_mask;
            g[hi] &= ~col_mask;
            r[lo] &= ~col_mask;
            r[hi] &= ~col_mask;

            for (uint8_t row = lo + 1; row < hi; row++) {
                g[row] &= ~((1 << lo) | (1 << hi));
                r[row] &= ~((1 << lo) | (1 << hi));
            }

            cli();
            memcpy((void *)green_buffer, g, 8);
            memcpy((void *)red_buffer,   r, 8);
            sei();
            _delay_ms(200);
        }

        _delay_ms(300);
    }
}

/* ==========================================================================
 * Timer2 compare-match ISR — display multiplexing
 *
 * Fires every ~1 ms.  For each row, we output two passes:
 *   1) red only
 *   2) green only
 * then advance to the next row.
 *
 * Shift order (last shifted = first register):
 *   1) red column byte   → lands in SR3
 *   2) green column byte → lands in SR2
 *   3) row select byte   → lands in SR1
 *   4) latch all three registers simultaneously
 * ========================================================================== */
ISR(TIMER2_COMPA_vect)
{
    /* Build the row-select byte: active-low, so only the current row is 0 */
    uint8_t row_byte = (uint8_t)~(1 << current_row);

    /* Call user-provided output function for red pass, then green pass */
    if (output_fn) {
        output_fn(row_byte,
                  0x00,
                  red_buffer[current_row]);

        output_fn(row_byte,
                  green_buffer[current_row],
                  0x00);
    }

    /* Advance to next row (wraps 0–7) */
    current_row = (current_row + 1) & 0x07;
}
