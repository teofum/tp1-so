#include <graphics.h>

#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>

#include <curses.h>

#define C_GRAY 20
#define CP_GRAYSCALE 1

#define CP_PLAYER 10

#define HEADER_HEIGHT 5
#define HEADER_MIN_WIDTH 18
#define HEADER_CAT_WIDTH 23

#define CELL_WIDTH 5
#define CELL_HEIGHT 3

#define GAME_OVER_WIDTH 44
#define GAME_OVER_HEIGHT 19

void gfx_init() {
  // TERM env var is lost when process is spawned from the provided master,
  // causing ncurses init to fail. We set it manually to work around this.
  setenv("TERM", "xterm-256color", 1);

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

static void clear_rect(int y1, int x1, int y2, int x2) {
  for (int y = y1; y <= y2; y++)
    for (int x = x1; x <= x2; x++)
      mvaddch(y, x, ' ');
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
  int player_is_here = value <= 0 && i == game_state->players[player_idx].y &&
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

static int compare_by_score(const void *a, const void *b) {
  const player_t *player_a = a;
  const player_t *player_b = b;

  // Sort descending by score
  int score_diff = (int)player_b->score - player_a->score;
  if (score_diff != 0)
    return score_diff;

  // Sort ascending by valid moves
  int valid_moves_diff =
      (int)player_a->requests_valid - player_b->requests_valid;
  if (valid_moves_diff != 0)
    return valid_moves_diff;

  // Sort ascending by invalid moves
  int invalid_moves_diff =
      (int)player_a->requests_invalid - player_b->requests_invalid;
  return invalid_moves_diff;
}

void draw_game_over(game_state_t *game_state) {
  uint32_t x1 = (COLS - GAME_OVER_WIDTH) >> 1;
  uint32_t y1 = (LINES - GAME_OVER_HEIGHT) >> 1;
  uint32_t x2 = x1 + GAME_OVER_WIDTH - 1;
  uint32_t y2 = y1 + GAME_OVER_HEIGHT - 1;

  attr_set(A_NORMAL, 0, NULL);

  // Draw window border and clear interior
  clear_rect(y1, x1, y2, x2);
  rect(y1, x1, y2, x2);

  // Draw text
  move(y1 + 2, x1 + (GAME_OVER_WIDTH - 9) / 2);
  printw("Game Over");

  // Get winner and draw it
  player_t players[MAX_PLAYERS];
  for (int i = 0; i < game_state->n_players; i++) {
    players[i] = game_state->players[i];

    // Abuse pid field to store the index, easier than making a new struct
    players[i].pid = i;
  }
  qsort(players, game_state->n_players, sizeof(player_t), compare_by_score);

  // Find player index of winner
  // Not very efficient, but this only runs once so whatever, extra couple
  // microseconds won't make a difference
  int winner_idx = players[0].pid;

  attr_set(A_NORMAL, CP_PLAYER + winner_idx, NULL);
  move(y1 + 3, x1 + (GAME_OVER_WIDTH - 14) / 2);
  printw("Player %d wins!", winner_idx + 1);

  attr_set(A_NORMAL, 0, NULL);
  move(y1 + 5, x1 + (GAME_OVER_WIDTH - 30) / 2);
  printw("# Player   Score Valid Invalid");

  for (int i = 0; i < game_state->n_players; i++) {
    attr_set(A_NORMAL, CP_PLAYER + players[i].pid, NULL);
    move(y1 + 6 + i, x1 + (GAME_OVER_WIDTH - 30) / 2);
    printw("%d %-8s %5d %5d %7d", players[i].pid + 1, players[i].name,
           players[i].score, players[i].requests_valid,
           players[i].requests_invalid);
  }

  attr_set(A_NORMAL, 0, NULL);
  move(y2 - 2, x1 + (GAME_OVER_WIDTH - 21) / 2);
  printw("Press any key to exit");
}
