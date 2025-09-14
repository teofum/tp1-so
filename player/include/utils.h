#ifndef UTILS_H
#define UTILS_H

#include <game_state.h>

/*
 * Get a direction number from a dx/dy pair
 */
char to_move(int dx, int dy);

/*
 * Get dx from a direction number
 */
int dx(int move);

/*
 * Get dy from a direction number
 */
int dy(int move);

/*
 * Check that a given position in the board is in bounds.
 * Returns 1 if in bounds, 0 if out of bounds.
 */
int check_bounds(int x, int y, game_state_t *gs);

/*
 * Check that a given position in the board is available (in bounds and empty).
 * Returns 1 if available, 0 if not.
 */
int available(int x, int y, game_state_t *gs);

#endif
