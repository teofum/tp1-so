#include <args.h>
#include <game.h>
#include <graphics.h>

#include <signal.h>
#include <stdio.h>
#include <stdlib.h>

#include <curses.h>

game_t game = NULL;

void cleanup() {
  if (game)
    game_disconnect(game);
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

  /*
   * Connect to game
   */
  game = game_connect(width, height);
  if (!game) {
    printf("View: failed to connect to game\n");
    return -1;
  }

  // Make sure to disconnect the game on exit and on common signal termination
  atexit(cleanup);
  signal(SIGTERM, cleanup);
  signal(SIGSEGV, cleanup);
  signal(SIGINT, cleanup);

  // Signal to master view is ready and started successfully
  game_post_view_ready(game);

  game_state_t *state = game_state(game);

  /*
   * Init ncurses
   */
  gfx_init();

  /*
   * Main loop
   */
  int game_running = 1;
  while (game_running) {
    game_wait_view_should_update(game);

    erase();

    for (int i = 0; i < state->n_players; i++)
      draw_player_card(i, state);

    draw_grid(state);

    if (state->game_ended) {
      draw_game_over(state);
      game_running = 0;
    }

    refresh();

    game_post_view_did_update(game);
  }

  getch();

  // Cleanup ncurses
  endwin();

  return 0;
}
