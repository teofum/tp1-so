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

static void rect(int y1, int x1, int y2, int x2) {
  mvhline(y1, x1, 0, x2 - x1);
  mvhline(y2, x1, 0, x2 - x1);
  mvvline(y1, x1, 0, y2 - y1);
  mvvline(y1, x2, 0, y2 - y1);
  mvaddch(y1, x1, ACS_ULCORNER);
  mvaddch(y2, x1, ACS_LLCORNER);
  mvaddch(y1, x2, ACS_URCORNER);
  mvaddch(y2, x2, ACS_LRCORNER);
}

static void get_cell_contents(char *buf, int value, int i, int j,
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

void draw_cell(int i, int j, game_state_t *game_state) {
  static char buf[15];
  int value = game_state->board[i * game_state->board_width + j];
  int color_pair = value > 0 ? value : CP_PLAYER - value;

  get_cell_contents(buf, value, i, j, game_state);
  attr_set(A_NORMAL, color_pair, NULL);
  rect(i * 3, j * 5, i * 3 + 2, j * 5 + 4);
  mvaddstr(i * 3 + 1, j * 5 + 1, buf);
}
