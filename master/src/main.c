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
 */ // en teoria esta bien
int make_move(int player, char dir, game_state_t *game_state) {
  //printf("d:%d",dir);
  int x = game_state->players[player].x;
  int y = game_state->players[player].y;

  int mx=0, my=0;

  if (dir==7 || dir==0 || dir==1) {
    ++my;
  } else if (dir==3 || dir==4 || dir==5 ) {
    --my;
  }
  if (dir==1 || dir==2 || dir==3) {
    ++mx;
  } else if (dir==5 || dir==6 || dir==7) {
    --mx;
  }

  int curpos = (game_state->board_width * y + x);
  //printf("c:{%d,%d}",x,y);
  int newpos = (curpos + (game_state->board_width * my + mx));
  //printf("n:{%d,%d}||",x+mx,y+my);

  // check if valid //todo esto esta mal
  if (!(0 <= (x + mx) && (x + mx) < game_state->board_width) ||
      !(0 <= (y + my) && (y + my) < game_state->board_height) ||
      game_state->board[newpos] <= 0) {
    ++game_state->players[player].requests_invalid;
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
  /*game_state->board_height = args.height;
  game_state->board_width = args.width;
  game_state->n_players = 1337;
  for (int i = 0; i < game_state->board_height; i++) {
    for (int j = 0; j < game_state->board_width; j++) {
      game_state->board[i * game_state->board_width + j] = i + j - 8;
    }
  }*/
  logpid();
  printf("Initializing game state...\n");
  game_state_init(game_state, &args);

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

    char child_argv[20][2];
    sprintf(child_argv[0], "%u", game_state->board_width);
    sprintf(child_argv[1], "%u", game_state->board_height);

  for (int i = 0; args.players[i] != NULL; i++) {
    players[i] = spawn_player(args.players[i], child_argv);
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
   * Process player move requests mk2
   */

  // Initialize semaphores in game_sync // 0 locks and 1 unlocks
  sem_init(&game_sync->view_should_update, 1, 0); // 0: view waits for master
  sem_init(&game_sync->view_did_update, 1, 0);    // 0: master waits for view

  sem_init(&game_sync->master_write_mutex, 1, 1); // 1: unlocked
  sem_init(&game_sync->game_state_mutex, 1, 1);   // 1: unlocked
  sem_init(&game_sync->read_count_mutex, 1, 1);   // 1: unlocked

  for(int i=0; i < game_state->n_players ; ++i){
    sem_init(&game_sync->player_may_move[i],1,0); // 0: players waits for master
  }

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
      // todo: esto solo lee 0 
      //printf("read{%d}\n",buf);
      sem_post(&game_sync->player_may_move[current_player]);// todo : pregunta, esto puede que tenga que ir antes

      //printf("red:%d form P{%d}||",buf, current_player);//solo lee 0
      if(make_move(current_player, buf, game_state)){ 
        // Valid move
        gettimeofday(&end, NULL);
        elapsed_s = (end.tv_sec - start.tv_sec) + (end.tv_usec - start.tv_usec) / 1000000.0;
        if(elapsed_s > args.timeout){//timed out
          game_state->game_ended = 1;
        }else{
          gettimeofday(&start, NULL);
        }
      }else{
        // Invalid move
        gettimeofday(&end, NULL);
        elapsed_s = (end.tv_sec - start.tv_sec) + (end.tv_usec - start.tv_usec) / 1000000.0;
        if(elapsed_s > args.timeout){ //timed out
          game_state->game_ended = 1;
        }
      }

      sem_post(&game_sync->game_state_mutex);

      // view //
      sem_post(&game_sync->view_should_update);
      sem_wait(&game_sync->view_did_update);

      usleep(args.delay);
    }
     // Current_player todavia no esta listo para lectura
     // veo el proximo
    current_player = (current_player + 1) % game_state->n_players;
  }

  // Done with semaphores
  sem_destroy(&game_sync->view_should_update);
  sem_destroy(&game_sync->view_did_update);

  sem_destroy(&game_sync->master_write_mutex);
  sem_destroy(&game_sync->game_state_mutex);
  sem_destroy(&game_sync->read_count_mutex);

  int ret;
  waitpid(view_pid, &ret, 0);
  logpid();
  printf("Waiting for view process to end...\n");
  logpid();
  printf("View process exited with code %d\n", ret);

  // Todo: el proceso de salir del view anda raro
  logpid();
  printf("Game ended (╯°□°）╯︵ ┻━┻ \n");


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
  printf("Unlinking shared memory...\n");
  shm_unlink("/game_state");

  // TODO: free the args once we get the shtuff into shmem
  free_args(&args);

  logpid();
  printf("Bye!\n");
  return 0;
}

