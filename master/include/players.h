#ifndef PLAYERS_H
#define PLAYERS_H

#include <game.h>
#include <move.h>

/*
 * Players ADT
 */
typedef struct players_cdt_t *players_t;

/*
 * Callback function to run some code when a player exits during
 * players_wait_all function call
 */
typedef void (*player_wait_callback_t)(player_t *player, int ret);

/*
 * Create a players object, spawn all player processes and configure pipes.
 * Initializes player order (round-robin).
 */
players_t players_create(game_state_t *game_state, args_t *args);

/*
 * Checks whether the next player in the round order has sent a move. The index
 * of the next player is written to next_player.
 * If there is a move, the function returns a positive integer and the move is
 * written to move. If the player has not sent a move, the return value is zero
 * and the value of move is undefined.
 * If there is an error, the function returns a negative value and errno is set.
 */
int players_next(players_t players, uint32_t *next_player, move_t *move);

/*
 * Returns 1 if all players are blocked, otherwise returns 0.
 */
int players_all_blocked(players_t players);

/*
 * Wait for all player processes to end, close their respective fds and destroy
 * the players object.
 * Optionally accepts a function to be called upon each player exiting.
 */
void players_wait_all(players_t players, player_wait_callback_t callback);

#endif
