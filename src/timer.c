/* ==========================================================================
 * timer.c — Timer peripheral configuration
 * EE3463 Final Project — 3D Snek :3
 *
 * Timer0  8-bit CTC  ~10 ms    debounce sampling    (ISR in input.c)
 * Timer1 16-bit CTC  variable  game tick             (ISR here)
 * Timer2  8-bit CTC  ~1 kHz    display multiplexing  (ISR in display.c)
 * ========================================================================== */

#include <avr/io.h>
#include <avr/interrupt.h>
#include "config.h"
#include "timer.h"

/* ---------- shared flag ------------------------------------------------- */
volatile uint8_t game_tick_flag = 0;

/* ==========================================================================
 * timer_init — configure all three timers, interrupts NOT yet enabled
 *              globally (call sei() in main after all inits).
 * ========================================================================== */
void timer_init(void)
{
    /* ------------------------------------------------------------------
     * TIMER 0 — CTC mode, prescaler 1024
     * Period = (OCR0A + 1) * 1024 / 16 000 000
     * OCR0A = 155  →  (156 * 1024) / 16e6 = 9.984 ms  ≈ 10 ms
     * ------------------------------------------------------------------ */
    TCCR0A = (1 << WGM01);                       /* CTC mode (TOP = OCR0A) */
    TCCR0B = (1 << CS02) | (1 << CS00);           /* clk/1024              */
    OCR0A  = 155;
    TIMSK0 = (1 << OCIE0A);                       /* enable compare-match A interrupt */

    /* ------------------------------------------------------------------
     * TIMER 1 — CTC mode, prescaler 256
     * Default period = 400 ms  →  OCR1A = 24999
     * ------------------------------------------------------------------ */
    TCCR1A = 0;
    TCCR1B = (1 << WGM12) | (1 << CS12);          /* CTC, clk/256          */
    OCR1A  = TICK_SLOW;
    TIMSK1 = (1 << OCIE1A);                       /* enable compare-match A interrupt */

    /* ------------------------------------------------------------------
     * TIMER 2 — CTC mode, prescaler 64
     * Period = (OCR2A + 1) * 64 / 16 000 000
     * OCR2A = 249  →  (250 * 64) / 16e6 = 1.0 ms  =  1 kHz
     * ------------------------------------------------------------------ */
    TCCR2A = (1 << WGM21);                        /* CTC mode              */
    TCCR2B = (1 << CS22);                          /* clk/64                */
    OCR2A  = 249;
    TIMSK2 = (1 << OCIE2A);                       /* enable compare-match A interrupt */
}

/* ==========================================================================
 * timer_set_tick_period — change the game-tick speed mid-game
 * ========================================================================== */
void timer_set_tick_period(uint16_t ocr_value)
{
    /* Reset counter so the new period takes effect immediately. */
    TCNT1  = 0;
    OCR1A  = ocr_value;
}

/* ==========================================================================
 * Timer1 compare-match ISR — fires every game tick (400 ms or 250 ms).
 * We ONLY set a flag; all heavy work happens in the main loop.
 * ========================================================================== */
ISR(TIMER1_COMPA_vect)
{
    game_tick_flag = 1;
}
