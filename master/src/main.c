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
  printf("Hello world!\n\n");

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

  // TODO: test code remove later
  game_state->board_height = args.height;
  game_state->board_width = args.width;
  game_state->n_players = 1337;
  for (int i = 0; i < game_state->board_height; i++) {
    for (int j = 0; j < game_state->board_width; j++) {
      game_state->board[i * game_state->board_width + j] = i + j - 8;
    }
  }

  /*
   * Fork and exec the view process
   */
  logpid();
  printf("Spawning view process...\n");
  int view_pid = fork();
  if (view_pid == -1) {
    logpid();
    printf("Failed to fork view process\n");
    return -1;
  } else if (!view_pid) {
    execv(args.view, argv);
  }

  /*
   * Fork and exec player processes
   */
  logpid();
  printf("Spawning player processes...\n");
  player_data_t players[MAX_PLAYERS];

  for (int i = 0; args.players[i] != NULL; i++) {
    players[i] = spawn_player(args.players[i], argv);
  }

  logpid();
  printf("Player communication test...\n");
  char buf[256];
  for (int i = 0; args.players[i] != NULL; i++) {
    dprintf(players[i].pipe_tx, "Hello player %d", i + 1);
    int read_bytes = read(players[i].pipe_rx, buf, 256);
    buf[read_bytes] = 0;

    logpid();
    printf("Player %d response: %s\n", i + 1, buf);
  }

  /*
   * Process player move requests mk1
   */

  // Initialize semaphores in game_sync // 0 locks and 1 unlocks
  sem_init(&game_sync->view_should_update, 1, 0); // 0: view waits for master
  sem_init(&game_sync->view_did_update, 1, 0);    // 0: master waits for view

  sem_init(&game_sync->master_write_mutex, 1, 1); // 1: unlocked
  sem_init(&game_sync->game_state_mutex, 1, 1);   // 1: unlocked
  sem_init(&game_sync->read_count_mutex, 1, 1);   // 1: unlocked

  // Select setup
  fd_set current_pipe;
  
  struct timeval timeout_zero = { 0, 0 };// para que revise instantaneamente
  int current_player = 0;

  // Timeout 
  struct timeval start, end;
  long elapsed_s;
  gettimeofday(&start, NULL);
  
  while (!game_state->game_ended) {

    FD_ZERO(&current_pipe); // vacia el fd_set
    FD_SET(players[current_player].pipe_rx,&current_pipe);
    
    int res = select( players[current_player].pipe_rx+1, &current_pipe, NULL, NULL, &timeout_zero);
    if ( res < 0 ){ // Error
      logpid();
      printf("Select error :( \n");
      return -1;
    }else if( res > 0 ){ // current_player listo para lectura
      sem_wait(&game_sync->master_write_mutex);
      sem_wait(&game_sync->game_state_mutex);
      sem_post(&game_sync->master_write_mutex);

      // ejecutar movimiento //
      char buf;
      read(players[current_player].pipe_tx, &buf, 1);
      if(make_move(current_player, buf, game_state)){ 
        // Valid move
        gettimeofday(&end, NULL);
        
        elapsed_s = (end.tv_sec - start.tv_sec) + (end.tv_usec - start.tv_usec) / 1000000.0;
        if(elapsed_s > args.timeout){
          game_state->game_ended = 1;
          logpid();
          printf("Valid move wait Timed out. Ending game.\n");
        }else{
          gettimeofday(&start, NULL);
        }
      }

      sem_post(&game_sync->game_state_mutex);

      // view //
      sem_post(&game_sync->view_should_update);
      sem_wait(&game_sync->view_did_update);
      sem_wait(&game_sync->view_should_update);

      usleep(args.delay);
    }
     // Current_player todavia no esta listo para lectura
     // veo el proximo
    current_player = (current_player + 1) % MAX_PLAYERS;
  }

  // Done with semaphores
  sem_destroy(&game_sync->view_should_update);
  sem_destroy(&game_sync->view_did_update);

  sem_destroy(&game_sync->master_write_mutex);
  sem_destroy(&game_sync->game_state_mutex);
  sem_destroy(&game_sync->read_count_mutex);


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

  logpid();
  printf("Waiting for view process to end...\n");
  int ret;
  waitpid(view_pid, &ret, 0);
  logpid();
  printf("View process exited with code %d\n", ret);

  logpid();
  printf("Unlinking shared memory...\n");
  shm_unlink("/game_state");

  // TODO: free the args once we get the shtuff into shmem
  free_args(&args);

  logpid();
  printf("Bye!\n");
  return 0;
}
