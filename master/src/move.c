#include <move.h>
#include <utils.h>

/*
 * Returns 1 if the player has any valid moves available, 0 if not.
 */
static int can_move(game_state_t *game_state, int player_idx) {
  int can_move = 0;

  player_t *player = &game_state->players[player_idx];
  for (int i = player->y - 1; i <= player->y + 1 && !can_move; i++) {
    for (int j = player->x - 1; j <= player->x + 1 && !can_move; j++) {
      if (available(j, i, game_state))
        can_move = 1;
    }
  }

  return can_move;
}

/*
 * Attempts to make a move. If the move is valid, updates game state and
 * returns 1. If it is invalid, returns 0.
 */
static int attempt_move(game_state_t *game_state, int player_idx, move_t move) {
  if (move < MOVE_UP || move > MOVE_UP_LEFT) {
    return 0;
  }

  int x = game_state->players[player_idx].x;
  int y = game_state->players[player_idx].y;

  int mx = dx(move), my = dy(move);

  int current_idx = game_state->board_width * y + x;
  int new_idx = current_idx + (game_state->board_width * my + mx);

  if (!available(x + mx, y + my, game_state)) {
    return 0;
  }

  game_state->players[player_idx].score += game_state->board[new_idx];
  game_state->board[new_idx] = -player_idx;

  game_state->players[player_idx].x = x + mx;
  game_state->players[player_idx].y = y + my;

  return 1;
}

int process_move(game_t game, int player_idx, move_t move) {
  game_state_t *state = game_state(game);
  player_t *player = &state->players[player_idx];

  game_lock_state_for_writing(game);

  // Process move
  int valid = attempt_move(state, player_idx, move);

  if (valid) {
    player->requests_valid++;
  } else {
    player->requests_invalid++;
  }

  // Block player if it can't make valid moves
  if (!can_move(state, player_idx)) {
    player->blocked = 1;
  }

  game_post_move_processed(game, player_idx);

  game_release_state(game);

  return valid;
}
