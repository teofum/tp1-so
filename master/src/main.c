#include <game.h>
#include <move.h>
#include <players.h>
#include <spawn.h>
#include <timeout.h>
#include <view.h>

#include <stdio.h>
#include <sys/wait.h>

/*
 * Main function
 */
int main(int argc, char **argv) {
  const char *err;

  // Parse args
  args_t args;
  if (!parse_args(argc, argv, &args, &err)) {
    free_args(&args);
    fprintf(stderr, "Parse error: %s\n", err);
    return -1;
  }

  // Initialize game
  game_t game = game_init(&args, &err);
  if (!game) {
    free_args(&args);
    fprintf(stderr, "Game initialization failed: %s\n", err);
    return -1;
  }

  // Get a pointer to the game state for convenience
  game_state_t *state = game_state(game);

  /*
   * Spawn view and players
   */
  view_t view = view_create(game, &args);
  if (!view) {
    fprintf(stderr, "View initialization failed\n");
    return -1;
  }

  players_t players = players_create(game, &args);
  if (!players) {
    fprintf(stderr, "Player initialization failed\n");
    return -1;
  }

  /*
   * Timeout
   */
  timeout_t timeout = timeout_create(sec_to_micros(args.timeout));
  free_args(&args);

  /*
   * Process player move requests until game ends
   */
  while (!state->game_ended) {
    player_move_t next_move = players_next(players, timeout);
    if (next_move.error) {
      perror("Players next failed");
      return -1;
    }

    if (next_move.will_move) {
      if (process_move(game, next_move.player_idx, next_move.move)) {
        timeout_reset(timeout);
      }
    }

    view_update(view);

    if (players_all_blocked(players) || timeout_check(timeout))
      game_end(game);
  }

  /*
   * Wait for child processes and clean up resources
   */
  view_wait(view, NULL);
  players_wait_all(players, NULL);
  game_destroy(game);

  return 0;
}
