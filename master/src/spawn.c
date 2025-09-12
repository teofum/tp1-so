#include <spawn.h>
#include <stdio.h>
#include <unistd.h>

static int exec_with_board_size(const char *path, game_state_t *game_state) {
  static char child_argv[2][10];
  sprintf(child_argv[0], "%u", game_state->board_width);
  sprintf(child_argv[1], "%u", game_state->board_height);

  return execl(path, path, child_argv[0], child_argv[1], NULL);
}

int spawn_player(const char *path_to_executable, game_state_t *game_state,
                 size_t i) {
  int player_pipe[2];

  // Create pipe to communicate with player
  if (pipe(player_pipe) == -1) {
    return -1; // Failed to create pipe
  }

  pid_t pid = fork();
  if (!pid) {
    // Map stdout to player -> master pipe
    dup2(player_pipe[1], 1);

    // Close unused fds from previously spawned players
    // player doesn't need any fds other than stdin/stdout/stderr
    for (int i = 3; i <= player_pipe[1]; i++) {
      close(i);
    }

    int res = exec_with_board_size(path_to_executable, game_state);
    if (res < 0) {
      perror("Player exec failed");

      // Set the player as blocked from the start if the executable failed tt
      // start, reference impl does this
      game_state->players[i].blocked = 1;

      exit(-1);
    }
  }

  // Close unused write end fd (master)
  close(player_pipe[1]);

  // Return read end
  game_state->players[i].pid = pid;
  return player_pipe[0];
}

pid_t spawn_view(const char *path_to_executable, game_state_t *game_state) {
  pid_t pid = fork();
  if (!pid) {
    int res = exec_with_board_size(path_to_executable, game_state);
    if (res < 0) {
      perror("View exec failed");
      exit(-1);
    }
  }

  return pid;
}
