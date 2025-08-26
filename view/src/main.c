#include <game_state.h>
#include <game_sync.h>
#include <shm_utils.h>

#include <stdio.h>
#include <sys/fcntl.h>
#include <sys/mman.h>
#include <sys/stat.h>
#include <sys/wait.h>
#include <unistd.h>

void logpid() { printf("[view: %d] ", getpid()); }

int main(int argc, char **argv) {
  logpid();
  printf("Hello world\n");

  /*
   * Set up shared memory
   */
  game_state_t *game_state =
      shm_open_and_map("/game_state", O_RDONLY, sizeof(game_state_t));
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

  logpid();
  printf("n_players is %d\n", game_state->n_players);

  return 0;
}
