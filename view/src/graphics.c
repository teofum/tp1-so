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

static void cat(int y1, int x1, int y2, int x2) {
  mvhline(y1, x1 + 1, 0, x2 - x1 - 2);
  mvhline(y2, x1, 0, x2 - x1);
  mvvline(y1, x1, 0, y2 - y1);
  mvvline(y1, x2, 0, y2 - y1);
  mvaddch(y1, x1 + 1, '^');
  mvaddch(y1, x2 - 1, '^');
  mvaddch(y1, x1, ACS_ULCORNER);
  mvaddch(y2, x1, ACS_LLCORNER);
  mvaddch(y1, x2, ACS_URCORNER);
  mvaddch(y2, x2, ACS_LRCORNER);
}

static void get_cell_contents(char *buf, int value, int i, int j,
                              game_state_t *game_state) {
  int player_idx = -value;
  int player_is_here = i == game_state->players[player_idx].y &&
                       j == game_state->players[player_idx].x;

  if (value > 0) {
    sprintf(buf, " %d ", value);
  } else if (player_is_here) {
    // this is super inefficient! but computers are fast :)
    int blocked = 1;
    for (int ii = i - 1; ii < i + 2; ii++) {
      for (int jj = j - 1; jj < j + 2; jj++) {
        if (ii < 0 || ii >= game_state->board_height || jj < 0 ||
            jj > game_state->board_width)
          continue;
        int local_value = game_state->board[ii * game_state->board_width + jj];
        if (local_value > 0)
          blocked = 0;
      }
    }

    sprintf(buf, blocked ? "-.-" : "o.o");
  } else {
    sprintf(buf, "   ");
  }
}

void draw_cell(int i, int j, game_state_t *game_state) {
  static char buf[15];
  int value = game_state->board[i * game_state->board_width + j];
  int color_pair = value > 0 ? value : CP_PLAYER - value;

  int player_idx = -value;
  int player_is_here = value <= 0 && i == game_state->players[player_idx].y &&
                       j == game_state->players[player_idx].x;

  get_cell_contents(buf, value, i, j, game_state);
  attr_set(A_NORMAL, color_pair, NULL);

  uint32_t max_players_per_row = COLS / HEADER_MIN_WIDTH;
  uint32_t off =
      (game_state->n_players + max_players_per_row - 1) / max_players_per_row;

  if (player_is_here) {
    cat(y1(i, off), x1(j), y2(i, off), x2(j));
  } else {
    rect(y1(i, off), x1(j), y2(i, off), x2(j));
  }
  mvaddstr(y1(i, off) + 1, x1(j) + 1, buf);
}

void draw_player_card(int player_idx, game_state_t *game_state) {
  player_t player = game_state->players[player_idx];

  // We actually need to do these calculations every frame, as the terminal
  // size may change
  uint32_t max_players_per_row = COLS / HEADER_MIN_WIDTH;
  uint32_t n_rows =
      (game_state->n_players + max_players_per_row - 1) / max_players_per_row;
  uint32_t players_per_row = (game_state->n_players + n_rows - 1) / n_rows;

  uint32_t row_idx = player_idx / players_per_row;
  uint32_t col_idx = player_idx % players_per_row;
  uint32_t card_width = COLS / players_per_row;

  uint32_t x0 = col_idx * card_width;
  uint32_t y0 = row_idx * HEADER_HEIGHT;
  uint32_t x1 = x0;
  uint32_t y1 = y0;
  uint32_t x2 = x1 + card_width - 1;
  uint32_t y2 = y1 + HEADER_HEIGHT - 1;

  int color_pair = CP_PLAYER + player_idx;
  attr_set(A_NORMAL, color_pair, NULL);

  // if we have enough room, draw a cat for each player
  static char buf[15];
  if (card_width > 23) {
    x1 += CELL_WIDTH;
  }

  rect(y1, x1, y2, x2);
  move(y1 + 1, x1 + 1);
  printw("%8s%s", player.name, player.blocked ? " [BLOCK]" : "");
  move(y1 + 2, x1 + 1);
  printw("Score: %d", player.score);
  move(y1 + 3, x1 + 1);
  printw("Moves: %d/%d", player.requests_valid, player.requests_invalid);

  if (card_width > 23) {
    get_cell_contents(buf, -player_idx, player.y, player.x, game_state);
    cat(y1 + 1, x0, y2 - 1, x0 + CELL_WIDTH - 1);
    mvaddstr(y1 + 2, x0 + 1, buf);
  }
}
