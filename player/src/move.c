#include <move.h>
#include <stdlib.h>
#include <utils.h>
#include "queue.h"

/* ==========================================================================
 * Simple/Naive strategies
 * ========================================================================== */
#ifdef STRAT_BLIND

/*
 * "Blind" strategy
 * The dumbest player possible, makes moves at random with no regard for what's
 * on the board around it.
 */
char get_next_move(game_state_t *game_state, int player_idx, char last_move) {
  return rand() % 8;
}

#endif

#ifdef STRAT_NAIVE

/*
 * "Naive" strategy
 * A very simple player: makes moves at random, only checking it won't move
 * into an occupied space. If there are no available moves, kills itself.
 */
char get_next_move(game_state_t *game_state, int player_idx, char last_move) {
  int x = game_state->players[player_idx].x;
  int y = game_state->players[player_idx].y;

  char move = rand() % 8;
  char initial = move;
  while (!available(x + dx(move), y + dy(move), game_state)) {
    move = (move + 1) % 8;
    if (move == initial) {
      return -1; // All directions are blocked
    }
  }

  return move;
}

#endif

/* ==========================================================================
 * Wall-following functions
 * ========================================================================== */
#ifdef STRAT_WALL

#ifdef WALL_L

static char wallhug(game_state_t *game_state, int player_idx, char prev) {
  int x = game_state->players[player_idx].x;
  int y = game_state->players[player_idx].y;

  char check = (prev + 6) % 8;
  if (available(x + dx(check), y + dy(check), game_state))
    return check;

  for (char next = prev; next < prev + 8; ++next) {
    char move = next % 8;
    if (available(x + dx(move), y + dy(move), game_state)) {
      return move;
    }
  }

  return -1;
}

#else

static char wallhug(game_state_t *game_state, int player_idx, char prev) {
  int x = game_state->players[player_idx].x;
  int y = game_state->players[player_idx].y;

  prev += 2 * 8;
  char check = (prev + 2) % 8;
  if (available(x + dx(check), y + dy(check), game_state))
    return check;

  for (char next = prev; next > prev - 8; --next) {
    char move = next % 8;
    if (available(x + dx(move), y + dy(move), game_state)) {
      return move;
    }
  }

  return -1;
}

#endif

#endif

/* ==========================================================================
 * Greedy strategies
 * ========================================================================== */
#ifdef STRAT_GREEDY

/* ==========================================================================
 * Greedy algorithm scoring functions
 * ========================================================================== */
#ifdef GREEDY_VALUE_WEIGHTED_L

/*
 * "Weighted Greedy Large" strategy value function
 * A smarter greedy algorithm. Checks a 5x5 box around the adjacent cell for
 * the one with the highest sum value, weighing the cells closer to the center
 * higher.
 * This scoring function assumes input is in bounds and available.
 */
static int scoring_fn(game_state_t *gs, int x, int y) {
  // Define Gaussian kernel
  static int kernel[3][3] = {{1, 4, 7}, {4, 16, 26}, {7, 26, 41}};

  int sum = 0;
  int blocked = 0;

  for (int dy = -2; dy <= 2; ++dy) {
    for (int dx = -2; dx <= 2; ++dx) {
      if (available(x + dx, y + dy, gs)) {
        int value = gs->board[(x + dx) + (y + dy) * gs->board_width];
        sum += (value * value) * kernel[2 - abs(dy)][2 - abs(dx)];
      } else if (dy > -2 && dy < 2 && dx > -2 && dx < 2) {
        blocked++;
      }
    }
  }

  if (blocked == 8)
    return 1;

  return sum;
}

#else
#ifdef GREEDY_VALUE_WEIGHTED

/*
 * "Weighted Greedy" strategy value function
 * A smarter greedy algorithm. Checks a 3x3 box around the adjacent cell for
 * the one with the highest sum value, weighing the cells closer to the center
 * higher.
 * This scoring function assumes input is in bounds and available.
 */
static int scoring_fn(game_state_t *gs, int x, int y) {
  // Define Gaussian kernel
  static int kernel[3][3] = {{1, 2, 1}, {2, 4, 2}, {1, 2, 1}};

  int sum = 0;
  int blocked = 0;

  for (int dy = -1; dy <= 1; ++dy) {
    for (int dx = -1; dx <= 1; ++dx) {
      if (available(x + dx, y + dy, gs)) {
        int value = gs->board[(x + dx) + (y + dy) * gs->board_width];
        sum += (value * value) * kernel[dy + 1][dx + 1];
      } else {
        blocked++;
      }
    }
  }

  if (blocked == 8)
    return 1;

  return sum;
}

#else
#ifdef GREEDY_VALUE_BOX

/*
 * "Box Greedy" strategy value function
 * A somewhat smarter greedy algorithm. Checks a 3x3 box around the adjacent
 * cell for the one with the highest sum value.
 * This scoring function assumes input is in bounds and available.
 */
static int scoring_fn(game_state_t *gs, int x, int y) {
  int sum = 0;
  int blocked = 0;

  for (int dy = -1; dy <= 1; ++dy) {
    for (int dx = -1; dx <= 1; ++dx) {
      if (available(x + dx, y + dy, gs)) {
        sum += gs->board[(x + dx) + (y + dy) * gs->board_width];
      } else {
        blocked++;
      }
    }
  }

  if (blocked == 8)
    return 1;

  return sum;
}

#else

/*
 * "Greedy" strategy value function
 * A simple greedy algorithm. Finds the adjacent cell with the highest value
 * and moves there.
 * This scoring function assumes input is in bounds and available.
 */
static int scoring_fn(game_state_t *gs, int x, int y) {
  return gs->board[x + y * gs->board_width];
}

#endif
#endif
#endif

/* ==========================================================================
 * Greedy algorithm move functions
 * ========================================================================== */
#ifdef GREEDY_STOCHASTIC

/*
 * Move function for stochastic greedy strats. Chooses a move direction at
 * random, weighing the probabilities of each direction by the value returned
 * by the scoring function.
 * Returns -1 (kills itself) if all neighbors are occupied.
 */
static char calculate_move(game_state_t *gs, int x, int y) {
  int values_cumulative[8]; // Cumulative probability function
  int values_sum = 0;       // Total

  for (int m = 0; m < 8; m++) {
    int value = 0;

    if (available(x + dx(m), y + dy(m), gs)) {
      value = scoring_fn(gs, x + dx(m), y + dy(m));
    }

    values_cumulative[m] = values_sum + value;
    values_sum += value;
  }

  if (values_sum == 0)
    return -1; // All adjacent cells are occupied, die

  int r = rand() % values_sum;
  for (int move = 0; move < 8; move++) {
    if (r < values_cumulative[move])
      return (char)move;
  }

  return 7; // We should never get here
}

#else

/*
 * Move function for greedy strats. Returns the direction towards the most
 * valuable adjacent cell given a scoring function.
 * Returns -1 (kills itself) if all neighbors are occupied.
 */
static char calculate_move(game_state_t *gs, int x, int y) {
  char move = -1;
  int max_value = -1;

  for (int dy = -1; dy <= 1; ++dy) {
    for (int dx = -1; dx <= 1; ++dx) {
      if ((dx != 0 || dy != 0) && available(x + dx, y + dy, gs)) {
        int value = scoring_fn(gs, x + dx, y + dy);
        if (value > max_value) {
          move = to_move(dx, dy);
          max_value = value;
        }
      }
    }
  }

  return move;
}

#endif

/*
 * Universal next move function for all greedy strats.
 */
char get_next_move(game_state_t *game_state, int player_idx, char last_move) {
  int x = game_state->players[player_idx].x;
  int y = game_state->players[player_idx].y;

  return calculate_move(game_state, x, y);
}

#endif

/* ==========================================================================
 * Wall-following strategy
 * ========================================================================== */
#if defined STRAT_WALL

char get_next_move(game_state_t *game_state, int player_idx, char last_move) {
  static int hit_wall = 0;

  int x = game_state->players[player_idx].x;
  int y = game_state->players[player_idx].y;

  if (last_move < 0) {
    // First move: pick the direction with the shortest path to a map edge
    int bw = game_state->board_width;
    int bh = game_state->board_height;

    int dx = x < bw / 2 ? -1 : 1;
    int dy = y < bh / 2 ? -1 : 1;

    int min_dist_x = dx < 0 ? x : bw - x - 1;
    int min_dist_y = dy < 0 ? y : bh - y - 1;

    return min_dist_x < min_dist_y ? to_move(dx, 0) : to_move(0, dy);
  }

  // Keep moving in the initial direction until we hit a wall
  if (!hit_wall) {
    if (available(x + dx(last_move), y + dy(last_move), game_state)) {
      return last_move;
    } else {
      hit_wall = 1;
    }
  }

  // After a wall hit, stay close to the wall
  return wallhug(game_state, player_idx, last_move);
}

#endif

/* ==========================================================================
 * Blocker strategy
 * ========================================================================== */
#ifdef STRAT_BLOCKER

/*
 * BFS to find shortest distance to target opponent.
 * Returns the minimum number of moves needed to reach target position.
 */
static int bfs_distance(game_state_t *gs, int x, int y, int target_x, int target_y) {
  if (x == target_x && y == target_y)
    return 0;

  int size = gs->board_width * gs->board_height;
  int *visited = calloc(size, sizeof(int));
  if (!visited)
    return 1000;

  queue_t q;
  queue_init(&q);
  queue_enqueue(&q, (int2_t){x, y});
  visited[x + y * gs->board_width] = 1;

  int dist = 0;
  int2_t curr;
  while (queue_dequeue(&q, &curr)) {
    dist++;

    for (int d = 0; d < 8; ++d) {
      int nx = curr.x + dx(d);
      int ny = curr.y + dy(d);
      int idx = nx + ny * gs->board_width;

      if (nx == target_x && ny == target_y) {
        free(visited);
        return dist;
      }

      if (available(nx, ny, gs) && !visited[idx]) {
        queue_enqueue(&q, (int2_t){nx, ny});
        visited[idx] = 1;
      }
    }
  }

  free(visited);
  return 1000;
}

/*
 * "Blocker" strategy value function
 * A blocking algorithm. Finds the adjacent cell that minimizes distance to
 * the nearest opponent using BFS pathfinding.
 * This scoring function assumes input is in bounds and available.
 */
static int scoring_fn(game_state_t *gs, int x, int y, int player_idx) {
  int min_dist = 1000;

  for (int i = 0; i < gs->n_players; ++i) {
    if (i != player_idx && !gs->players[i].blocked) {
      int opponent_x = gs->players[i].x;
      int opponent_y = gs->players[i].y;
      int dist = bfs_distance(gs, x, y, opponent_x, opponent_y);
      if (dist < min_dist)
        min_dist = dist;
    }
  }

  // No opponents found, use greedy behavior
  if (min_dist == 1000)
    return gs->board[x + y * gs->board_width];

  return 1000 - min_dist;
}

/*
 * Move function for blocker strat. Returns the direction towards the closest
 * opponent to block their movement options.
 * Returns -1 (kills itself) if all neighbors are occupied.
 */
static char calculate_move(game_state_t *gs, int x, int y, int player_idx) {
  char move = -1;
  int max_value = -1;

  for (int dy = -1; dy <= 1; ++dy) {
    for (int dx = -1; dx <= 1; ++dx) {
      if ((dx != 0 || dy != 0) && available(x + dx, y + dy, gs)) {
        int value = scoring_fn(gs, x + dx, y + dy, player_idx);
        if (value > max_value) {
          move = to_move(dx, dy);
          max_value = value;
        }
      }
    }
  }

  return move;
}

char get_next_move(game_state_t *game_state, int player_idx, char last_move) {
  int x = game_state->players[player_idx].x;
  int y = game_state->players[player_idx].y;

  return calculate_move(game_state, x, y, player_idx);
}

#endif
