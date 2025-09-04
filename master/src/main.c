#include <args.h>
#include <game_state.h>
#include <game_sync.h>
#include <shm_utils.h>
#include <spawn.h>

#include <stdio.h>
#include <sys/fcntl.h>
#include <sys/mman.h>
#include <sys/stat.h>
#include <sys/wait.h>
#include <unistd.h>

#include <semaphore.h> // TODO ; esto noc si va aca
#include <sys/time.h>  // TODO ; esto noc si va aca

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
  /*
   * Parse command line args
   */
  args_t args;
  const char *parse_err = NULL;
  if (!parse_args(argc, argv, &args, &parse_err)) {
    free_args(&args);
    printf("Failed to parse args: %s\n", parse_err);
    return -1;
  }

  /*
   * Print the args we received
   * TODO: remove this debug code
   */
  logpid();
  printf("Board size %ux%u\n", args.width, args.height);
  logpid();
  printf("Delay %ums\n", args.delay);
  logpid();
  printf("Timeout %us\n", args.timeout);
  logpid();
  printf("Seed %d\n", args.seed);

  if (args.view) {
    logpid();
    printf("View executable %s\n", args.view);
  }

  for (int i = 0; args.players[i] != NULL; i++) {
    logpid();
    printf("Player %d: %s\n", i + 1, args.players[i]);
  }

  printf("\n");

  /*
   * Set up shared memory
   */
  game_state_t *game_state =
      shm_open_and_map("/game_state", O_RDWR | O_CREAT,
                       get_game_state_size(args.width, args.height));
  if (!game_state) {
    logpid();
    printf("Failed to create shared memory game_state\n");
    return -1;
  }

  game_sync_t *game_sync =
      shm_open_and_map("/game_sync", O_RDWR | O_CREAT, sizeof(game_sync_t));
  if (!game_state) {
    logpid();
    printf("Failed to create shared memory game_sync\n");
    return -1;
  }

  logpid();
  printf("Initializing game state...\n");
  game_state_init(game_state, &args);

  /*
   * Initialize semaphores
   */
  sem_init(&game_sync->view_should_update, 1, 0); // 0: view waits for master
  sem_init(&game_sync->view_did_update, 1, 0);    // 0: master waits for view

  sem_init(&game_sync->master_write_mutex, 1, 1); // 1: unlocked
  sem_init(&game_sync->game_state_mutex, 1, 1);   // 1: unlocked
  sem_init(&game_sync->read_count_mutex, 1, 1);   // 1: unlocked

  for (int i = 0; i < game_state->n_players; ++i) {
    sem_init(&game_sync->player_may_move[i], 1, 1);
  }

  /*
   * Fork and exec the view process
   */
  logpid();
  int view_pid = -1;
  if (args.view) {
    view_pid = fork();
    if (view_pid == -1) {
      logpid();
      printf("Failed to fork view process\n");
      return -1;
    } else if (!view_pid) {
      char child_argv[20][3];
      sprintf(child_argv[0], "%u", game_state->board_width);
      sprintf(child_argv[1], "%u", game_state->board_height);

      execl(args.view, args.view, child_argv[0], child_argv[1]);
    }
  } else {
    logpid();
    printf("No view process given, running headless...\n");
  }

  /*
   * Fork and exec player processes
   */
  player_data_t players[MAX_PLAYERS];
  for (int i = 0; i < game_state->n_players; i++) {
    players[i] = spawn_player(args.players[i], game_state);
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

  while (!game_state->game_ended) {
    FD_ZERO(&current_pipe); // vacia el fd_set
    FD_SET(players[current_player].pipe_rx, &current_pipe);

    // Allow player to move
    sem_post(&game_sync->player_may_move[current_player]);

    int res = select(players[current_player].pipe_rx + 1, &current_pipe, NULL,
                     NULL, &timeout_zero);
    if (res < 0) { // Error
      logpid();
      printf("Select error :( \n");
      return -1;
    } else if (res > 0) {
      // Take write lock and game state lock
      sem_wait(&game_sync->master_write_mutex);
      sem_wait(&game_sync->game_state_mutex);
      sem_post(&game_sync->master_write_mutex);

      // Read move
      char move;
      read(players[current_player].pipe_rx, &move, 1);

      // Process move
      if (make_move(current_player, move, game_state)) {
        gettimeofday(&start, NULL);
      } else {
        // Invalid move
        gettimeofday(&end, NULL);
        elapsed_s = end.tv_sec - start.tv_sec;
        if (elapsed_s > args.timeout) { // timed out
          game_state->game_ended = 1;
        }
      }

      // Block player if it can't make valid moves
      if (!player_can_move(game_state, current_player)) {
        game_state->players[current_player].blocked = 1;
      }

      // Release game state lock
      sem_post(&game_sync->game_state_mutex);

      // Signal view to update, wait for view and delay
      if (view_pid != -1) {
        sem_post(&game_sync->view_should_update);
        sem_wait(&game_sync->view_did_update);
      }

      usleep(args.delay * 10000);
    }

    current_player = (current_player + 1) % game_state->n_players;

    // If all players are blocked, end game
    int all_blocked = 1;
    for (int i = 0; i < game_state->n_players; i++) {
      if (!game_state->players[i].blocked) {
        all_blocked = 0;
        break;
      }
    }
    if (all_blocked)
      game_state->game_ended = 1;
  }

  // Habilita todo para que finalize
  sem_post(&game_sync->view_should_update);
  for (int i = 0; i < game_state->n_players; i++) {
    sem_post(&game_sync->player_may_move[i]);
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
  for (int i = 0; args.players[i] != NULL; i++) {
    int pid = players[i].pid;
    int ret;
    waitpid(pid, &ret, 0);
    logpid();
    printf("Player %d process with pid %d exited with code %d\n", i + 1, pid,
           ret);
  }

  // Done with semaphores
  sem_destroy(&game_sync->view_should_update);
  sem_destroy(&game_sync->view_did_update);

  sem_destroy(&game_sync->master_write_mutex);
  sem_destroy(&game_sync->game_state_mutex);
  sem_destroy(&game_sync->read_count_mutex);

  for (int i = 0; i < game_state->n_players; ++i) {
    sem_destroy(&game_sync->player_may_move[i]);
  }

  logpid();
  printf("Unlinking shared memory...\n");
  shm_unlink("/game_state");
  shm_unlink("/game_sync");

  // TODO: free the args once we get the shtuff into shmem
  free_args(&args);

  logpid();
  printf("Bye!\n");
  return 0;
}
