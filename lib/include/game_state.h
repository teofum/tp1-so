#ifndef GAME_STATE_H
#define GAME_STATE_H

#include <unistd.h>

#define MAX_PLAYERS 9

typedef struct {
  char name[16];
  unsigned int score;
  unsigned int requests_invalid;
  unsigned int requests_valid;
  unsigned short x, y;
  pid_t pid;
  bool blocked;
} player_t;

typedef struct {
  unsigned short board_width;
  unsigned short board_height;
  unsigned int n_players;
  player_t players[9];
  bool game_ended;
  int board[];
} game_state_t;

#endif
