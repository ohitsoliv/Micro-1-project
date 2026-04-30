/* ==========================================================================
 * input.h — Button reading with debounce and direction locking
 * EE3463 Final Project — 3D Snek :3
 * ========================================================================== */

#ifndef INPUT_H
#define INPUT_H

#include "game.h"   /* for Direction enum */

/* Initialise button pins (pull-ups) and debounce state.
 * Call once at startup AFTER timer_init(). */
void input_init(void);

/* Return the most recently validated direction.
 * Direction locking (can't reverse into yourself) is handled internally
 * using the snake's current heading.                                       */
Direction input_get_direction(void);

/* Tell the input module what direction the snake is currently moving so it
 * can enforce the "no 180° reversal" rule.  Called by game_update().       */
void input_set_current_direction(Direction d);

#endif /* INPUT_H */
