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

// TODO move this to files etc

int get_next_move() { return 0; }

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
    printf("Failed to create shared memory game_state\n");
    return -1;
  }

  game_sync_t *game_sync =
      shm_open_and_map("/game_sync", O_RDWR, sizeof(game_sync_t));
  if (!game_state) {
    logpid();
    printf("Failed to create shared memory game_sync\n");
    return -1;
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

    sem_wait(&game_sync->read_count_mutex);
    if (--game_sync->read_count == 0) {
      sem_post(&game_sync->game_state_mutex);
    }
    sem_post(&game_sync->read_count_mutex);

    int next_move = get_next_move();

    // Send next move to master
    sem_wait(&game_sync->player_may_move[0]);
    putchar(next_move);
  }

  return 0;
}
