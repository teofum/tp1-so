#ifndef MOVE_H
#define MOVE_H

#include <game.h>

typedef enum : char {
  MOVE_UP = 0,
  MOVE_UP_RIGHT,
  MOVE_RIGHT,
  MOVE_DOWN_RIGHT,
  MOVE_DOWN,
  MOVE_DOWN_LEFT,
  MOVE_LEFT,
  MOVE_UP_LEFT
} move_t;

/*
 * Process a move. Returns 1 if a valid move was processed, 0 otherwise.
 * Takes a writing lock on the game state and releases it when done.
 */
int process_move(game_t game, int player_idx, move_t move);

#endif
