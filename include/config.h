/* ==========================================================================
 * config.h — Pin definitions and project-wide constants
 * EE3463 Final Project — 3D Snek :3
 * Target: ATmega328P @ 16 MHz
 *
 * ALL hardware pins are defined here so the rest of the code never touches
 * raw port/pin names directly.
 * ========================================================================== */

#ifndef CONFIG_H
#define CONFIG_H

#include <avr/io.h>

/* ---------- CPU clock (must match Makefile / fuses) ---------- */
#ifndef F_CPU
#define F_CPU 16000000UL
#endif

/* ==========================================================================
 * BUTTONS — active-low with internal pull-ups, directly on PORTD
 * ========================================================================== */
#define BTN_DDR         DDRD
#define BTN_PORT        PORTD
#define BTN_PIN         PIND

#define BTN_UP_BIT      PD2
#define BTN_DOWN_BIT    PD3
#define BTN_LEFT_BIT    PD4
#define BTN_RIGHT_BIT   PD5

/* Masks for quick read / pull-up enable */
#define BTN_UP_MASK     (1 << BTN_UP_BIT)
#define BTN_DOWN_MASK   (1 << BTN_DOWN_BIT)
#define BTN_LEFT_MASK   (1 << BTN_LEFT_BIT)
#define BTN_RIGHT_MASK  (1 << BTN_RIGHT_BIT)
#define BTN_ALL_MASK    (BTN_UP_MASK | BTN_DOWN_MASK | BTN_LEFT_MASK | BTN_RIGHT_MASK)

/* ==========================================================================
 * DISPLAY OUTPUT PINS (74HC595 chain)
 * ========================================================================== */
#define SR_DDR          DDRB
#define SR_PORT         PORTB

#define SR_DATA_BIT     PB0
#define SR_CLK_BIT      PB1
#define SR_LATCH_BIT    PB2

#define SR_DATA_MASK    (1 << SR_DATA_BIT)
#define SR_CLK_MASK     (1 << SR_CLK_BIT)
#define SR_LATCH_MASK   (1 << SR_LATCH_BIT)

/* ==========================================================================
 * GAME CONSTANTS
 * ========================================================================== */
#define GRID_SIZE           8       /* 8×8 LED matrix                        */
#define MAX_SNAKE_LENGTH    64      /* worst case: entire grid               */
#define INITIAL_SNAKE_LEN   3       /* start with 3 segments                 */

#define LEVEL1_TREATS       5       /* treats to eat on level 1 to win       */
#define LEVEL2_TREATS       8       /* treats to eat on level 2 to win       */

/* Game tick periods (Timer1 OCR1A values, prescaler = 256, 16 MHz)
 *   Period = (OCR1A + 1) * 256 / 16 000 000
 *   400 ms → OCR1A = 24999
 *   250 ms → OCR1A = 15624                                                  */
#define TICK_SLOW           24999U  /* ~400 ms — level 1                     */
#define TICK_FAST           15624U  /* ~250 ms — level 2                     */

/* Debounce: 3 consecutive identical reads at 10 ms intervals               */
#define DEBOUNCE_COUNT      3

/* ADC channel for RNG seed (PC0, left floating)                            */
#define RNG_ADC_CHANNEL     0

#endif /* CONFIG_H */
