#ifndef PLAYERS_H
#define PLAYERS_H

#include <game.h>
#include <move.h>
#include <timeout.h>

/*
 * Players ADT
 */
typedef struct players_cdt_t *players_t;

/*
 * Move struct
 */
typedef struct {
  int error;
  int will_move;
  uint32_t player_idx;
  move_t move;
} player_move_t;

/*
 * Callback function to run some code when a player exits during
 * players_wait_all function call
 */
typedef void (*player_wait_callback_t)(player_t *player, uint32_t idx, int ret);

/*
 * Create a players object, spawn all player processes and configure pipes.
 * Initializes player order (round-robin).
 */
players_t players_create(game_t game, args_t *args);

/*
 * Checks whether the next player in the round order has sent a move.
 * Returns a struct containing information about the next move. If an error
 * happens, the struct "error" field is set to 1 and errno is set.
 */
player_move_t players_next(players_t players, timeout_t timeout);

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
