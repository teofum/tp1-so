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
    sprintf(buf, game_state->players[player_idx].blocked ? "-.-" : "o.o");
  } else {
    sprintf(buf, "   ");
  }
}

void draw_grid(game_state_t *game_state) {
  int32_t board_width = game_state->board_width * CELL_WIDTH;
  int32_t board_height = game_state->board_height * CELL_HEIGHT;

  uint32_t max_players_per_row = COLS / HEADER_MIN_WIDTH;
  uint32_t header_rows =
      (game_state->n_players + max_players_per_row - 1) / max_players_per_row;

  if (board_width > COLS ||
      board_height > LINES - HEADER_HEIGHT * header_rows) {
    for (int i = 0; i < game_state->board_height; i++) {
      for (int j = 0; j < game_state->board_width; j++) {
        draw_cell_mini(i, j, game_state);
      }
    }
  } else {
    for (int i = 0; i < game_state->board_height; i++) {
      for (int j = 0; j < game_state->board_width; j++) {
        draw_cell(i, j, game_state);
      }
    }
  }
}

void draw_cell(int i, int j, game_state_t *game_state) {
  // will only ever hold 3 chars, but it needs to not overflow if a large number
  // comes in for whatever reason
  static char buf[15];

  int value = game_state->board[i * game_state->board_width + j];
  int color_pair = value > 0 ? value : CP_PLAYER - value;

  int player_idx = -value;
  int player_is_here = value <= 0 && i == game_state->players[player_idx].y &&
                       j == game_state->players[player_idx].x;

  get_cell_contents(buf, value, i, j, game_state);
  attr_set(A_NORMAL, color_pair, NULL);

  uint32_t max_players_per_row = COLS / HEADER_MIN_WIDTH;
  uint32_t header_rows =
      (game_state->n_players + max_players_per_row - 1) / max_players_per_row;

  int32_t board_width = game_state->board_width * CELL_WIDTH;
  int32_t x_offset = (COLS - board_width) / 2;

  uint32_t y1 = i * CELL_HEIGHT + HEADER_HEIGHT * header_rows;
  uint32_t x1 = j * CELL_WIDTH + x_offset;
  uint32_t y2 = i * CELL_HEIGHT + CELL_HEIGHT - 1 + HEADER_HEIGHT * header_rows;
  uint32_t x2 = j * CELL_WIDTH + CELL_WIDTH - 1 + x_offset;

  if (player_is_here) {
    cat(y1, x1, y2, x2);
  } else {
    rect(y1, x1, y2, x2);
  }
  mvaddstr(y1 + 1, x1 + 1, buf);
}

void draw_cell_mini(int i, int j, game_state_t *game_state) {
  int value = game_state->board[i * game_state->board_width + j];
  int color_pair = value > 0 ? value : CP_PLAYER - value;

  int player_idx = -value;
  int player_is_here = value <= 0 && i == game_state->players[player_idx].y &&
                       j == game_state->players[player_idx].x;

  char cell_char = value > 0 ? value + '0' : player_is_here ? '#' : '+';
  attr_set(A_NORMAL, color_pair, NULL);

  uint32_t max_players_per_row = COLS / HEADER_MIN_WIDTH;
  uint32_t header_rows =
      (game_state->n_players + max_players_per_row - 1) / max_players_per_row;

  int32_t x_offset = (COLS - game_state->board_width) / 2;

  uint32_t y = i + HEADER_HEIGHT * header_rows;
  uint32_t x = j + x_offset;

  mvaddch(y, x, cell_char);
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
  printw("%-8s%s", player.name, player.blocked ? " [BLOCK]" : "");
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
