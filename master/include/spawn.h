#ifndef SPAWN_H
#define SPAWN_H

#include "game_state.h"
typedef struct {
  int pid;
  int pipe_rx;
} player_data_t;

player_data_t spawn_player(const char *path_to_executable,
                           game_state_t *game_state);

#endif
