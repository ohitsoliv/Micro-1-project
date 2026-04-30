/* ==========================================================================
 * timer.h — Timer peripherals for debounce, game tick, and display refresh
 * EE3463 Final Project — 3D Snek :3
 *
 * Timer0 — CTC ~10 ms   → button debounce sampling (ISR in input.c)
 * Timer1 — CTC variable  → game tick flag (400 ms layer 0 / 250 ms layer 1)
 * Timer2 — CTC ~1 kHz    → display row multiplexing (ISR in display.c)
 * ========================================================================== */

#ifndef TIMER_H
#define TIMER_H

#include <stdint.h>

/* Flag set by Timer1 ISR — main loop clears it after processing a tick. */
extern volatile uint8_t game_tick_flag;

/* Initialise all three timers.  Call once at startup before sei().         */
void timer_init(void);

/* Change Timer1 period (pass TICK_SLOW or TICK_FAST from config.h).       */
void timer_set_tick_period(uint16_t ocr_value);

#endif /* TIMER_H */
