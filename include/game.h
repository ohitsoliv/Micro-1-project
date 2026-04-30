/* ==========================================================================
 * game.h — Game state definitions and public interface
 * EE3463 Final Project — 3D Snek :3
 * ========================================================================== */

#ifndef GAME_H
#define GAME_H

#include <stdint.h>

/* ---------- A single cell on the 8×8 grid ---------- */
typedef struct {
    uint8_t x;  /* column 0-7 */
    uint8_t y;  /* row    0-7 */
} Point;

/* ---------- Movement directions ---------- */
typedef enum {
    DIR_UP = 0,
    DIR_DOWN,
    DIR_LEFT,
    DIR_RIGHT
} Direction;

/* ---------- Overall game state machine ---------- */
typedef enum {
    STATE_PLAYING,          /* normal gameplay                          */
    STATE_WIN_SEQUENCE,     /* level cleared — flash all LEDs           */
    STATE_LEVEL_TRANSITION, /* brief pause before starting next level   */
    STATE_GAME_OVER,        /* self-collision — red flash               */
    STATE_VICTORY           /* both levels cleared — victory animation  */
} GameState;

/* ---------- Public functions ---------- */

/* Initialise the game for layer 0.  Call once at power-on. */
void game_init(void);

/* Advance the game by one tick.  Call from main loop when game_tick_flag
 * is set.  Reads the current direction from the input module internally. */
void game_update(void);

/* Return the current high-level state so main.c can decide what to show. */
GameState game_get_state(void);

/* Return the current level number (0 or 1). */
uint8_t game_get_level(void);

/* Return the number of treats eaten on the current level. */
uint8_t game_get_treats_eaten(void);

/* ---- Direct access for the display renderer ---- */

/* Get pointer to the snake body array and its geometry so display.c can
 * render it without duplicating the data.                                */
const Point *game_get_body(void);
uint8_t      game_get_length(void);
uint8_t      game_get_tail_index(void);

/* Get the treat position. */
Point game_get_treat(void);

#endif /* GAME_H */
