#include <spawn.h>
#include <stdio.h>
#include <unistd.h>

player_data_t spawn_player(const char *path_to_executable,
                           game_state_t *game_state) {
  player_data_t player_data;
  int pipe_rx[2];

  // Create pipes to communicate with player
  if (pipe(pipe_rx) == -1) {
    // Failed to create pipes
    player_data.pid = -1;
    return player_data;
  }

  player_data.pid = fork();
  if (!player_data.pid) {
    // Map stdout to player -> master pipe
    dup2(pipe_rx[1], 1);

    // Close unused fd (player)
    close(pipe_rx[0]);
    // TODO: make sure we're not leaking any fds!

    char child_argv[20][2];
    sprintf(child_argv[0], "%u", game_state->board_width);
    sprintf(child_argv[1], "%u", game_state->board_height);

    execl(path_to_executable, path_to_executable, child_argv[0], child_argv[1]);
  }

  // Close unused fds (master)
  close(pipe_rx[1]);

  player_data.pipe_rx = pipe_rx[0];
  return player_data;
}
