#include <spawn.h>
#include <stdio.h>
#include <unistd.h>

static int exec_with_board_size(const char *path, game_state_t *game_state) {
  static char child_argv[20][2];
  sprintf(child_argv[0], "%u", game_state->board_width);
  sprintf(child_argv[1], "%u", game_state->board_height);

  return execl(path, path, child_argv[0], child_argv[1]);
}

int spawn_player(const char *path_to_executable, game_state_t *game_state,
                 size_t i) {
  int pipe_rx[2];

  // Create pipe to communicate with player
  if (pipe(pipe_rx) == -1) {
    return -1; // Failed to create pipe
  }

  pid_t pid = fork();
  if (!pid) {
    // Map stdout to player -> master pipe
    dup2(pipe_rx[1], 1);

    // Close unused fds from previously spawned players
    // player doesn't need any fds other than stdin/stdout/stderr
    for (int i = 3; i < pipe_rx[1]; i++) {
      close(i);
    }

    int res = exec_with_board_size(path_to_executable, game_state);
    // TODO this should exit if execl fails!!!
  }

  // Close unused write end fd (master)
  // For some reason, this causes all players to write to the same pipe
  // wtf??
  // close(pipe_rx[1]);

  // Return read end
  game_state->players[i].pid = pid;
  return pipe_rx[0];
}

pid_t spawn_view(const char *path_to_executable, game_state_t *game_state) {
  pid_t pid = fork();
  if (!pid) {
    int res = exec_with_board_size(path_to_executable, game_state);
    // TODO this should exit if execl fails!!!
  }

  return pid;
}
