/* ==========================================================================
 * main.c — Entry point and main game loop
 * EE3463 Final Project — 3D Snek :3
 * Target: ATmega328P @ 16 MHz
 *
 * Initialises all peripherals, then runs an infinite loop that:
 *   1. Waits for the game-tick flag (set by Timer1 ISR)
 *   2. Updates game logic (movement, collision, treats)
 *   3. Renders the display buffers
 *   4. Handles state transitions (win, game-over, layer change, victory)
 *
 * ALL heavy work happens here in the main loop context.  The ISRs only
 * set flags or shift out data that's already in the buffers.
 * ========================================================================== */

#include <avr/io.h>
#include <avr/interrupt.h>
#include <util/delay.h>

#include "config.h"
#include "timer.h"
#include "input.h"
#include "game.h"
#include "display.h"

/* ---------- functions defined in game.c but not in game.h --------------- */
extern void game_transition_to_layer2(void);
extern void game_restart(void);

/* ==========================================================================
 * main
 * ========================================================================== */
int main(void)
{
    /* ---- 1. Initialise everything (interrupts still disabled) ---- */
    display_init();         /* shift-register pins, blank the matrix     */
    input_init();           /* button pull-ups                           */
    timer_init();           /* Timer0/1/2 — CTC modes, interrupts armed  */
    game_init();            /* snake, treat, PRNG seed from ADC          */
    display_render();       /* draw the initial frame                    */

    /* ---- 2. Enable global interrupts ---- */
    sei();

    /* ---- 3. Main loop ---- */
    while (1) {
        /* Wait for the game-tick flag (set by Timer1 ISR every 250–400 ms).
         * The display ISR (Timer2) keeps refreshing the matrix in the
         * background while we sleep here.                                  */
        if (!game_tick_flag) {
            continue;
        }
        game_tick_flag = 0;

        /* ---- Handle current state ---- */
        switch (game_get_state()) {

            case STATE_PLAYING:
                game_update();
                display_render();
                break;

            case STATE_WIN_SEQUENCE:
                /* Player cleared layer 0 — flash yellow, then move to layer 1 */
                display_win_animation();

                /* Brief blank pause during transition */
                _delay_ms(500);

                game_transition_to_layer2();
                display_render();
                break;

            case STATE_LAYER_TRANSITION:
                /* (reserved — currently handled directly in WIN_SEQUENCE) */
                break;

            case STATE_GAME_OVER:
                /* Self-collision — flash red, then restart from layer 0 */
                display_game_over_animation();
                _delay_ms(500);

                game_restart();
                display_render();
                break;

            case STATE_VICTORY:
                /* Both layers cleared — fancy animation, then restart */
                display_victory_animation();
                _delay_ms(1000);

                game_restart();
                display_render();
                break;
        }
    }

    return 0;   /* never reached */
}
