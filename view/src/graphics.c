#include <graphics.h>

#include <stdint.h>
#include <stdlib.h>

#include <curses.h>

void gfx_init() {
  // TERM env var is lost when process is spawned from the provided master,
  // causing ncurses init to fail. We set it manually to work around this.
  setenv("TERM", "xterm-256color", 0);

  (void)initscr();
  (void)nonl();

  if (has_colors()) {
    start_color();

    for (int i = 0; i < 9; i++) {
      int16_t v = 300 + i * 50;
      init_color(C_GRAY + i, v, v, v);
      init_pair(CP_GRAYSCALE + i, C_GRAY + i, COLOR_BLACK);
    }

    for (int i = 0; i < 9; i++) {
      init_pair(CP_PLAYER + i, i + 1, COLOR_BLACK);
    }
  }
}

void get_cell_contents(char *buf, int value, int i, int j,
                       game_state_t *game_state) {
  int player_idx = -value;
  if (value > 0) {
    sprintf(buf, " %d ", value);
  } else if (i == game_state->players[player_idx].y &&
             j == game_state->players[player_idx].x) {
    int blocked = 1;
    for (int ii = i - 1; ii < i + 2; ii++) {
      for (int jj = j - 1; jj < j + 2; jj++) {
        int local_value = game_state->board[ii * game_state->board_width + jj];
        if (local_value > 0)
          blocked = 0;
      }
    }

    sprintf(buf, blocked ? "x_x" : "o_o");
  } else {
    sprintf(buf, "   ");
  }
}
