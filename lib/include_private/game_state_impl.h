/*
 * Game state private implementation.
 *
 * The game_state_t struct is made public so its members can be freely accessed
 * by calling code. However, these methods are only used internally so there's
 * no reason to expose them to the caller, so we make a private header.
 */

#ifndef GAME_STATE_IMPL_H
#define GAME_STATE_IMPL_H

#include "game_state.h"

/*
 * Get the game state struct size, given a board width and height.
 * The struct is dynamically sized due to the board size being unknown
 * at compile time.
 */
size_t get_game_state_size(uint16_t board_width, uint16_t board_height);

/*
 * Initialize game state with a board and player positions.
 * Used only in game init.
 */
void game_state_init(game_state_t *state, const args_t *args);

#endif
