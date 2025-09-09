#include <args.h>
#include <game_state.h>
#include <game_sync.h>
#include <shm_utils.h>

#include <semaphore.h>
#include <stdio.h>
#include <stdlib.h>
#include <sys/fcntl.h>
#include <sys/mman.h>
#include <sys/stat.h>
#include <sys/wait.h>
#include <unistd.h>

void logpid() { printf("[player: %d] ", getpid()); }
void logerr(const char *s) {
  fprintf(stderr, "[player: %d] %s\n", getpid(), s);
}

// TODO move this to files etc

// Convert diff 'x and y' into char dir
// !Asume que no hay dx=dy=0 que no deberia pasar
char getDir(int dx, int dy){
  char dirs[3][3] ={{7, 0,1},
                    {6,-1,2},
                    {5, 4,3}};

  return dirs[dy + 1][dx + 1];
}//2 = [1][2] // dy=0 dx=1

// returns 0 if out of bounds
int inBounds(int x,int y, game_state_t* gs){
  if( x < 0 || y < 0 || x >= gs->board_width || y >= gs->board_height){
  return 0;
  }
  return 1;
}

// en el gs tengo el player y el board
// LA POSICION DEL PLAYER NO SE UPDATEA PARA CUANDO ESTE ARRANCA A CORRER
char get_next_move(game_state_t* game_state, int player_idx) {
  char next=-1;
  int maxp=-1;

  int x = game_state->players[player_idx].x;
  int y = game_state->players[player_idx].y;

  for(int dy=-1; dy<=1; ++dy){
    for(int dx=-1; dx<=1; ++dx){
      if( inBounds( x + dx, y + dy, game_state ) ){
        int kernelIndex = ((x + dx)+((y + dy) * game_state->board_width ));
        if(game_state->board[kernelIndex]>maxp){
          
          next=getDir(dx,dy);
          maxp=game_state->board[kernelIndex];
        
        }
      }    
    }
  }
  
  return next;
}

int main(int argc, char **argv) {
  /*
   * Parse command line args and calculate size of game state
   */
  if (argc < 3) {
    return -1;
  }
  int width = atoi(argv[1]);
  int height = atoi(argv[2]);
  size_t game_state_size = get_game_state_size(width, height);

  /*
   * Set up shared memory
   */
  game_state_t *game_state =
      shm_open_and_map("/game_state", O_RDONLY, game_state_size);
  if (!game_state) {
    logpid();
    printf("Failed to open shared memory game_state\n");
    return -1;
  }

  game_sync_t *game_sync =
      shm_open_and_map("/game_sync", O_RDWR, sizeof(game_sync_t));
  if (!game_state) {
    logpid();
    printf("Failed to open shared memory game_sync\n");
    return -1;
  }

  int player_idx = 0;
  int pid = getpid();
  for (int i = 0; i < game_state->n_players; i++) {
    if (game_state->players[i].pid == pid) {
      player_idx = i;
      break;
    }
  }

  /*
   * Main loop
   */
  int running = 1;
  while (running) {
    // Sync with master to allow it to write to game state
    sem_wait(&game_sync->master_write_mutex);
    sem_post(&game_sync->master_write_mutex);

    // Lightswitch sync
    sem_wait(&game_sync->read_count_mutex);
    if (game_sync->read_count++ == 0) {
      sem_wait(&game_sync->game_state_mutex);
    }
    sem_post(&game_sync->read_count_mutex);

    // Read game state here

    // If the game ended or we're blocked, stop
    if (game_state->game_ended || game_state->players[player_idx].blocked) {
      running = 0;
    }

    sem_wait(&game_sync->read_count_mutex);
    if (--game_sync->read_count == 0) {
      sem_post(&game_sync->game_state_mutex);
    }
    sem_post(&game_sync->read_count_mutex);

    if (!running)
      break;

    wait(&game_sync->game_state_mutex);
    post(&game_sync->game_state_mutex);
    // Wait for master to update the board before computing next

    char next_move = get_next_move(game_state,player_idx);

    // Send next move to master
    sem_wait(&game_sync->player_may_move[player_idx]);
    write(STDOUT_FILENO, &next_move, 1);
  }

  shm_unlink("/game_state");
  shm_unlink("/game_sync");

  return 0;
}
