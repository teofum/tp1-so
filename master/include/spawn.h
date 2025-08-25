#ifndef SPAWN_H
#define SPAWN_H

typedef struct {
  int pid;
  int pipe_tx, pipe_rx;
} player_data_t;

player_data_t spawn_player(const char *path_to_executable, char *const *argv);

#endif
