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

  // check if valid //todo esto esta mal
  if (!(0 <= (x + mx) && (x + mx) < game_state->board_width) ||
      !(0 <= (y + my) && (y + my) < game_state->board_height) ||
      game_state->board[newpos] <= 0) {
    game_state->players[player].requests_invalid++;
    return 0;
  }

  ++game_state->players[player].requests_valid;

  game_state->players[player].score += game_state->board[newpos];
  game_state->board[newpos] = -player;

  game_state->players[player].x = x + mx;
  game_state->players[player].y = y + my;

  return 1;
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
  printf("Timeout %ums\n", args.timeout);
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
  logpid();
  printf("Creating shared memory...\n");

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
   * Fork and exec the view process
   */
  int view_pid = -1;
  if (args.view) {
    usleep(3000000);
    view_pid = fork();
    if (view_pid == -1) {
      logpid();
      printf("Failed to fork view process\n");
      return -1;
    } else if (!view_pid) {
      execv(args.view, argv);
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
  // Initialize semaphores in game_sync // 0 locks and 1 unlocks
  sem_init(&game_sync->view_should_update, 1, 0); // 0: view waits for master
  sem_init(&game_sync->view_did_update, 1, 0);    // 0: master waits for view

  sem_init(&game_sync->master_write_mutex, 1, 1); // 1: unlocked
  sem_init(&game_sync->game_state_mutex, 1, 1);   // 1: unlocked
  sem_init(&game_sync->read_count_mutex, 1, 1);   // 1: unlocked

  for (int i = 0; i < game_state->n_players; ++i) {
    sem_init(&game_sync->player_may_move[i], 1, 1);
  }

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
        // Valid move
        gettimeofday(&end, NULL);
        elapsed_s = (end.tv_sec - start.tv_sec) +
                    (end.tv_usec - start.tv_usec) / 1000000.0;
        if (elapsed_s > args.timeout) { // timed out
          game_state->game_ended = 1;
        } else {
          gettimeofday(&start, NULL);
        }
      } else {
        // Invalid move
        gettimeofday(&end, NULL);
        elapsed_s = (end.tv_sec - start.tv_sec) +
                    (end.tv_usec - start.tv_usec) / 1000000.0;
        if (elapsed_s > args.timeout) { // timed out
          game_state->game_ended = 1;
        }
      }

      // Allow player to move again
      sem_post(&game_sync->player_may_move[current_player]);

      // Release game state lock
      sem_post(&game_sync->game_state_mutex);

      // view //
      if (view_pid != -1) {
        sem_post(&game_sync->view_should_update);
        sem_wait(&game_sync->view_did_update);
      }

      usleep(args.delay * 1000);
    }
    // Current_player todavia no esta listo para lectura
    // veo el proximo
    current_player = (current_player + 1) % game_state->n_players;
  }
  // Habilita todo para que finalize
  sem_post(&game_sync->view_should_update);
  for (int i = 0; i < game_state->n_players; ++i) {
    sem_post(&game_sync->player_may_move[i]);
  }

  logpid();
  printf("Game ended (╯°□°）╯︵ ┻━┻ \n");

  // Done with semaphores
  sem_destroy(&game_sync->view_should_update);
  sem_destroy(&game_sync->view_did_update);

  sem_destroy(&game_sync->master_write_mutex);
  sem_destroy(&game_sync->game_state_mutex);
  sem_destroy(&game_sync->read_count_mutex);

  for (int i = 0; i < game_state->n_players; ++i) {
    sem_destroy(&game_sync->player_may_move[i]);
  }

  /*
   * Wait for child processes and clean up resources
   */
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

  if (view_pid != -1) {
    int ret;
    waitpid(view_pid, &ret, 0);
    logpid();
    printf("View process exited with code %d\n", ret);
  }

  logpid();
  printf("Unlinking shared memory...\n");
  shm_unlink("/game_state");

  // TODO: free the args once we get the shtuff into shmem
  free_args(&args);

  logpid();
  printf("Bye!\n");
  return 0;
}
