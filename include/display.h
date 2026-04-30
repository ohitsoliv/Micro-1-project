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

/* Initialise shift-register pins, layer select, and clear the buffers.
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
 * Used when the player beats both layers.                                  */
void display_victory_animation(void);

/* Set the physical layer select pin (0 = bottom, 1 = top). */
void display_set_layer(uint8_t layer);

#endif /* DISPLAY_H */
