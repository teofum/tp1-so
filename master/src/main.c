#include <game.h>
#include <move.h>
#include <spawn.h>
#include <timeout.h>

#include <stdio.h>

void logpid() { printf("[master: %d] ", getpid()); }

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
  } else {
    logpid();
    printf("No view process given, running headless...\n");
  }

  int player_pipes[MAX_PLAYERS];
  for (int i = 0; i < state->n_players; i++) {
    player_pipes[i] = spawn_player(args.players[i], state, i);
  }

  /*
   * Timeout
   */
  timeout_t timeout = timeout_create(args.timeout);
  free_args(&args);

  /*
   * Process player move requests
   */
  // Select setup
  fd_set current_pipe;
  struct timeval timeout_zero = {0}; // para que revise instantaneamente
  int current_player = 0;

  while (!state->game_ended) {
    FD_ZERO(&current_pipe); // vacia el fd_set
    FD_SET(player_pipes[current_player], &current_pipe);

    int res = select(player_pipes[current_player] + 1, &current_pipe, NULL,
                     NULL, &timeout_zero);
    if (res < 0) { // Error
      logpid();
      printf("Select error :( \n");
      return -1;
    } else if (res > 0) {
      // Read move
      char move;
      read(player_pipes[current_player], &move, 1);

      if (process_move(game, current_player, move)) {
        timeout_reset(timeout);
      }

      // Signal view to update, wait for view and delay
      if (view_pid != -1)
        game_update_view(game);

      usleep(args.delay * 10000);
    }

    current_player = (current_player + 1) % state->n_players;

    int all_blocked = 1;
    for (int i = 0; i < state->n_players; i++) {
      if (!state->players[i].blocked) {
        all_blocked = 0;
        break;
      }
    }

    if (all_blocked || timeout_check(timeout))
      game_end(game);
  }

  /*
   * Wait for child processes and clean up resources
   */
  if (view_pid != -1) {
    int ret;
    waitpid(view_pid, &ret, 0);
    logpid();
    printf("View process exited with code %d\n", ret);
  }

  logpid();
  printf("Waiting for child processes to end...\n");
  for (int i = 0; i < state->n_players; i++) {
    int pid = state->players[i].pid;
    int ret;
    waitpid(pid, &ret, 0);
    logpid();
    printf("Player %d with pid %d exited with code %d\n", i + 1, pid, ret);
  }

  game_destroy(game);

  logpid();
  printf("Bye!\n");
  return 0;
}
