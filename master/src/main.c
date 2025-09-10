#include <game.h>
#include <spawn.h>

#include <stdio.h>
#include <sys/time.h> // TODO ; esto noc si va aca
#include <sys/wait.h>

void logpid() { printf("[master: %d] ", getpid()); }

/*
 * Aplica el move, retorna 0 si fue invalido y 1 si se aplico
 */
int make_move(int player, char dir, game_state_t *game_state) {
  if (dir < 0 || dir > 7) {
    game_state->players[player].requests_invalid++;
    return 0;
  }

  int x = game_state->players[player].x;
  int y = game_state->players[player].y;

  int mx = 0, my = 0;

  if (dir == 7 || dir == 0 || dir == 1) {
    --my;
  } else if (dir == 3 || dir == 4 || dir == 5) {
    ++my;
  }
  if (dir == 1 || dir == 2 || dir == 3) {
    ++mx;
  } else if (dir == 5 || dir == 6 || dir == 7) {
    --mx;
  }

  int curpos = game_state->board_width * y + x;
  int newpos = curpos + (game_state->board_width * my + mx);

  if (!(0 <= (x + mx) && (x + mx) < game_state->board_width) ||
      !(0 <= (y + my) && (y + my) < game_state->board_height) ||
      game_state->board[newpos] <= 0) {
    game_state->players[player].requests_invalid++;
    return 0;
  }

  game_state->players[player].requests_valid++;

  game_state->players[player].score += game_state->board[newpos];
  game_state->board[newpos] = -player;

  game_state->players[player].x = x + mx;
  game_state->players[player].y = y + my;

  return 1;
}

int player_can_move(game_state_t *game_state, int player_idx) {
  int can_move = 0;

  player_t *player = &game_state->players[player_idx];
  for (int i = player->y - 1; i <= player->y + 1 && !can_move; i++) {
    for (int j = player->x - 1; j <= player->x + 1 && !can_move; j++) {
      if (i < 0 || j < 0 || i >= game_state->board_height ||
          j >= game_state->board_width)
        continue;

      if (game_state->board[i * game_state->board_width + j] > 0)
        can_move = 1;
    }
  }

  return can_move;
}

/*
 * Main function
 */
int main(int argc, char **argv) {
  const char *err;

  // Parse args
  args_t args;
  if (!parse_args(argc, argv, &args, &err)) {
    fprintf(stderr, "Parse error: %s", err);
    return -1;
  }

  // Initialize game
  game_t game = game_init(&args);
  if (!game) {
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
   * Process player move requests
   */
  // Select setup
  fd_set current_pipe;
  struct timeval timeout_zero = {0}; // para que revise instantaneamente
  int current_player = 0;

  // Timeout
  struct timeval start, end;
  long elapsed_s;
  gettimeofday(&start, NULL);

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
      game_lock_state_for_writing(game);

      // Read move
      char move;
      read(player_pipes[current_player], &move, 1);

      if (state->players[current_player].blocked) {
        printf("blocked player %d moved!!! %d\n", current_player, (int)move);
      }

      // Process move
      if (make_move(current_player, move, state)) {
        gettimeofday(&start, NULL);
      } else {
        // Invalid move
        gettimeofday(&end, NULL);
        elapsed_s = end.tv_sec - start.tv_sec;
        if (elapsed_s > args.timeout) {
          game_end(game);
        }
      }

      // Block player if it can't make valid moves
      if (!player_can_move(state, current_player)) {
        state->players[current_player].blocked = 1;
      }

      game_post_move_processed(game, current_player);

      game_release_state(game);

      // Signal view to update, wait for view and delay
      if (view_pid != -1)
        game_update_view(game);

      usleep(args.delay * 10000);
    }

    current_player = (current_player + 1) % state->n_players;

    // If all players are blocked, end game
    int all_blocked = 1;
    for (int i = 0; i < state->n_players; i++) {
      if (!state->players[i].blocked) {
        all_blocked = 0;
        break;
      }
    }

    if (all_blocked)
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

  // TODO: free the args once we get the shtuff into shmem
  free_args(&args);

  logpid();
  printf("Bye!\n");
  return 0;
}
