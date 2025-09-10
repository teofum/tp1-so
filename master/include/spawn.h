#ifndef SPAWN_H
#define SPAWN_H

#include <game_state.h>

int spawn_player(const char *path_to_executable, game_state_t *game_state,
                 size_t i);

pid_t spawn_view(const char *path_to_executable, game_state_t *game_state);

#endif
