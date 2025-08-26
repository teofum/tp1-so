#include <args.h>
#include <game_state.h>
#include <game_sync.h>
#include <shm_utils.h>

#include <stdio.h>
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
  printf("Hello world\n");
  logpid();
  printf("Board size %ux%u\n", args.width, args.height);

  size_t game_state_size = get_game_state_size(args.width, args.height);

  free_args(&args);

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

  // temporary shitty board print
  // TODO: make this better
  char buf[5];
  for (int i = 0; i < game_state->board_height; i++) {
    for (int j = 0; j < game_state->board_width; j++) {
      int value = game_state->board[i * game_state->board_width + j];
      sprintf(buf, "%02d ", value);
      attr_set(A_NORMAL, value > 0 ? 0 : -value, NULL);
      mvaddstr(i * 2, j * 3, buf);
    }
    printf("\n");
  }

  getch();

  // Cleanup ncurses
  endwin();

  return 0;
}
