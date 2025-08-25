#include <spawn.h>
#include <unistd.h>

player_data_t spawn_player(const char *path_to_executable, char *const *argv) {
  player_data_t player_data;
  int pipe_tx[2], pipe_rx[2];

  // Create pipes to communicate with player
  if (pipe(pipe_tx) == -1 || pipe(pipe_rx) == -1) {
    // Failed to create pipes
    player_data.pid = -1;
    return player_data;
  }

  player_data.pid = fork();
  if (!player_data.pid) {
    // Connect player stdin to master -> player pipe
    dup2(pipe_tx[0], 0);
    // And stdout to player -> master pipe
    dup2(pipe_rx[1], 1);

    // Close unused fds (player)
    close(pipe_tx[1]);
    close(pipe_rx[0]);
    // TODO: make sure we're not leaking any fds!

    execv(path_to_executable, argv);
  }

  // Close unused fds (master)
  close(pipe_tx[0]);
  close(pipe_rx[1]);

  player_data.pipe_tx = pipe_tx[1];
  player_data.pipe_rx = pipe_rx[0];

  return player_data;
}
