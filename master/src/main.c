#include <game.h>
#include <move.h>
#include <players.h>
#include <spawn.h>
#include <sys/wait.h>
#include <timeout.h>

#include <stdio.h>

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
   * Fork and exec processes
   */
  int view_pid = -1;
  if (args.view) {
    view_pid = spawn_view(args.view, state);
  }

  players_t players = players_create(state, &args);

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

      // Signal view to update, wait for view and delay
      if (view_pid != -1)
        game_update_view(game);

      usleep(args.delay * 10000);
    }

    if (players_all_blocked(players) || timeout_check(timeout))
      game_end(game);
  }

  /*
   * Wait for child processes and clean up resources
   */
  if (view_pid != -1) {
    int ret;
    waitpid(view_pid, &ret, 0);
  }

  players_wait_all(players, NULL);
  game_destroy(game);

  return 0;
}
