#include <args.h>
#include <game_state.h>
#include <game_sync.h>
#include <graphics.h>
#include <shm_utils.h>

#include <semaphore.h>
#include <stdio.h>
#include <stdlib.h>
#include <sys/fcntl.h>
#include <sys/mman.h>
#include <sys/stat.h>
#include <sys/wait.h>
#include <unistd.h>

#include <curses.h>

void logpid() { printf("[view: %d] ", getpid()); }

int main(int argc, char **argv) {
  /*
   * Parse command line args and calculate size of game state
   */
  if (argc < 3) {
    return -1;
  }
  int width = atoi(argv[1]);
  int height = atoi(argv[2]);

  /*
   * Print the args we received
   * TODO: remove this debug code
   */
  logpid();
  printf("Hello world\n");
  logpid();
  printf("Board size %ux%u\n", width, height);

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
   * Init ncurses
   */
  gfx_init();

  /*
   * Main loop
   */
  int game_running = 1;
  while (game_running) {
    sem_wait(&game_sync->view_should_update);

    for (int i = 0; i < game_state->board_height; i++) {
      for (int j = 0; j < game_state->board_width; j++) {
        draw_cell(i, j, game_state);
      }
    }
    refresh();

    sem_post(&game_sync->view_did_update);

    if (game_state->game_ended) {
      game_running = 0;
    }
  }

  // Cleanup ncurses
  endwin();

  return 0;
}
