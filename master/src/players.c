#include <players.h>
#include <spawn.h>

#include <sys/wait.h>

struct players_cdt_t {
  int pipe_fds[MAX_PLAYERS];

  fd_set current_fds;

  uint32_t current_player_idx;
  game_state_t *game_state;
};

players_t players_create(game_state_t *game_state, args_t *args) {
  players_t players = malloc(sizeof(struct players_cdt_t));
  if (!players)
    return NULL;

  for (int i = 0; i < game_state->n_players; i++) {
    players->pipe_fds[i] = spawn_player(args->players[i], game_state, i);
  }

  players->current_player_idx = 0;
  players->game_state = game_state;

  return players;
}

int players_next(players_t players, uint32_t *next_player, move_t *move) {
  *next_player = players->current_player_idx;

  struct timeval timeout_zero = {0}; // Nonblocking select
  int next_player_pipe = players->pipe_fds[*next_player];
  fd_set *fds = &players->current_fds;

  // Empty fd set and add the current player's pipe fd
  FD_ZERO(fds);
  FD_SET(next_player_pipe, fds);

  int res = select(next_player_pipe + 1, fds, NULL, NULL, &timeout_zero);
  if (res < 0)
    return res;

  if (res > 0) {
    char c_move; // Read to a char because C enums are always 4 bytes
    read(next_player_pipe, &c_move, 1);
    *move = c_move;
  }

  players->current_player_idx =
      (players->current_player_idx + 1) % players->game_state->n_players;
  return res;
}

void players_wait_all(players_t players, player_wait_callback_t callback) {
  int ret;
  for (int i = 0; i < players->game_state->n_players; i++) {
    player_t *player = &players->game_state->players[i];
    int pid = player->pid;
    waitpid(pid, &ret, 0);

    close(players->pipe_fds[i]);

    if (callback)
      callback(player, ret);
  }

  free(players);
}
