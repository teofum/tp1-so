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
    fprintf(stderr, "Parse error: %s", err);
    return -1;
  }

  // Initialize game
  game_t game = game_init(&args);
  if (!game) {
    free_args(&args);
    fprintf(stderr, "%s", err);
    return -1;
  }

  // Get a pointer to the game state for convenience
  game_state_t *state = game_state(game);

  /*
   * Spawn view and players
   */
  view_t view = view_create(game, &args);
  players_t players = players_create(game, &args);

  /*
   * Timeout
   */
  timeout_t timeout = timeout_create(args.timeout);
  free_args(&args);

  /*
   * Process player move requests until game ends
   */
  uint32_t current_player;
  move_t move;
  while (!state->game_ended) {
    int player_will_move = players_next(players, &current_player, &move);

    if (player_will_move < 0) {
      perror("Select failed: ");
      return -1;
    } else if (player_will_move) {
      if (process_move(game, current_player, move)) {
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
