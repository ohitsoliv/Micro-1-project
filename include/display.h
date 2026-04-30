/* ==========================================================================
 * display.h — 8×8 bicolor LED matrix driver (74HC595 shift registers)
 * EE3463 Final Project — 3D Snek :3
 *
 * Two frame buffers (green_buffer and red_buffer) are continuously shifted
 * out by a Timer2 ISR at ~1 kHz, one row per interrupt.
 * ========================================================================== */

#ifndef DISPLAY_H
#define DISPLAY_H

#include <stdint.h>

/* Frame buffers — written by display_render(), read by the Timer2 ISR.
 * Each byte represents one row; bit N = column N.                         */
extern volatile uint8_t green_buffer[8];
extern volatile uint8_t red_buffer[8];

/* Output callback — called from the Timer2 ISR (~1 kHz base).
 * For each row, the ISR calls this callback twice:
 *   1) red pass   (green_cols = 0, red_cols = row red data)
 *   2) green pass (green_cols = row green data, red_cols = 0)
 * row_select : active-low byte, only the active row's bit is 0.
 * The callback MUST be ISR-safe (fast, no blocking, no malloc). */
typedef void (*display_output_fn)(uint8_t row_select,
                                  uint8_t green_cols,
                                  uint8_t red_cols);

/* Register the hardware output callback before calling display_init().
 * Without a registered callback the display will be blank. */
void display_set_output_fn(display_output_fn fn);

/* Initialise the display driver and clear the frame buffers.
 * Call once at startup.                                                    */
void display_init(void);

/* Convert the current game state (snake body + treat) into the frame
 * buffers.  Call once per game tick from the main loop.                    */
void display_render(void);

/* Blocking animation: flash all LEDs yellow (red+green) three times.
 * Used when the player clears a layer.                                    */
void display_win_animation(void);

/* Blocking animation: flash all LEDs red three times.
 * Used on game-over (self-collision).                                      */
void display_game_over_animation(void);

/* Blocking animation: cascading spiral in both colours.
 * Used when the player beats both levels.                                  */
void display_victory_animation(void);

#endif /* DISPLAY_H */
