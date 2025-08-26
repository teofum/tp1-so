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
   * Init ncurses stuff
   * TODO: clean this shit up
   */

  // TERM env var is lost when process is spawned from the provided master,
  // causing ncurses init to fail. We set it manually to work around this.
  setenv("TERM", "xterm", 0);

  (void)initscr();
  (void)nonl();

  if (has_colors()) {
    start_color();

    init_pair(1, COLOR_RED, COLOR_BLACK);
    init_pair(2, COLOR_GREEN, COLOR_BLACK);
    init_pair(3, COLOR_YELLOW, COLOR_BLACK);
    init_pair(4, COLOR_BLUE, COLOR_BLACK);
    init_pair(5, COLOR_CYAN, COLOR_BLACK);
    init_pair(6, COLOR_MAGENTA, COLOR_BLACK);
    init_pair(7, COLOR_WHITE, COLOR_BLACK);
  }

  int game_running = 1;
  while (game_running) {
    sem_wait(&game_sync->view_should_update);

    // temporary shitty board print
    // TODO: make this better
    char buf[5];
    for (int i = 0; i < game_state->board_height; i++) {
      for (int j = 0; j < game_state->board_width; j++) {
        int value = game_state->board[i * game_state->board_width + j];
        sprintf(buf, "%02d ", value);
        attr_set(A_NORMAL, value > 0 ? 0 : -value + 1, NULL);
        mvaddstr(i * 2, j * 3, buf);
      }
    }

    sem_post(&game_sync->view_did_update);

    if (game_state->game_ended) {
      game_running = 0;
      // TODO: print "press any key to exit" message
      getch();
    }
  }

  // Cleanup ncurses
  endwin();

  return 0;
}
