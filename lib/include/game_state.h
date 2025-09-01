#ifndef GAME_STATE_H
#define GAME_STATE_H

#include <stdint.h>
#include <unistd.h>
#include <stdlib.h>
#include <string.h>

#include <args.h>

#define MAX_PLAYERS 9
#define MAX_PLAYER_NAME 16

typedef struct {
  char name[MAX_PLAYER_NAME];
  uint32_t score;
  uint32_t requests_invalid;
  uint32_t requests_valid;
  uint16_t x, y;
  pid_t pid;
  int32_t blocked;
} player_t;

typedef struct {
  uint16_t board_width;
  uint16_t board_height;
  uint32_t n_players;
  player_t players[MAX_PLAYERS];
  int32_t game_ended;
  int32_t board[];
} game_state_t;

size_t get_game_state_size(uint16_t board_width, uint16_t board_height);

void game_state_init(game_state_t *state, const args_t *args);

#endif

