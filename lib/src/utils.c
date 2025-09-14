#include <utils.h>

char to_move(int dx, int dy) {
  static char moves[3][3] = {{7, 0, 1}, {6, -1, 2}, {5, 4, 3}};

  return moves[dy + 1][dx + 1];
}

int dx(int move) {
  static int x[8] = {0, 1, 1, 1, 0, -1, -1, -1};
  return x[move];
}

int dy(int move) {
  static int y[8] = {-1, -1, 0, 1, 1, 1, 0, -1};
  return y[move];
}

int check_bounds(int x, int y, game_state_t *gs) {
  return (x >= 0 && y >= 0 && x < gs->board_width && y < gs->board_height);
}

int available(int x, int y, game_state_t *gs) {
  return check_bounds(x, y, gs) && gs->board[x + y * gs->board_width] > 0;
}
