#include <players.h>
#include <spawn.h>

#include <sys/wait.h>

struct players_cdt_t {
  int pipe_fds[MAX_PLAYERS];

  fd_set current_fds;

  uint32_t current_player_idx;
  uint32_t last_select_player_idx;
  game_state_t *game_state;
};

players_t players_create(game_t game, args_t *args) {
  players_t players = malloc(sizeof(struct players_cdt_t));
  if (!players)
    return NULL;

  game_state_t *state = game_state(game);
  for (int i = 0; i < state->n_players; i++) {
    players->pipe_fds[i] = spawn_player(args->players[i], state, i);
  }

  players->current_player_idx = 0;
  players->last_select_player_idx = 0;
  players->game_state = state;

  FD_ZERO(&players->current_fds);

  return players;
}

/*
 * Returns 1 if the fd_set used for tracking player pipes is empty
 */
static int players_fdset_empty(players_t players) {
  uint32_t n_players = players->game_state->n_players;

  for (int i = 0; i < n_players; i++) {
    if (FD_ISSET(players->pipe_fds[i], &players->current_fds))
      return 0;
  }

  return 1;
}

/*
 * Call the select syscall to get all players that have moves to make
 */
static int players_select(players_t players) {
  uint32_t n_players = players->game_state->n_players;

  // Fill fdset with fds of all players' pipes
  for (int i = 0; i < n_players; i++) {
    FD_SET(players->pipe_fds[i], &players->current_fds);
  }

  // Do select syscall to get ready players
  int max_pipe = players->pipe_fds[n_players - 1];
  struct timeval timeout_zero = {0}; // Nonblocking select
  return select(max_pipe + 1, &players->current_fds, NULL, NULL, &timeout_zero);
}

int players_next(players_t players, uint32_t *next_player, move_t *move) {
  *next_player = players->current_player_idx;
  int next_player_pipe = players->pipe_fds[*next_player];

  int res;

  // If the fdset is empty we need to call select
  // On error, return error and let the caller handle it
  if (players_fdset_empty(players)) {
    res = players_select(players);
    if (res < 0)
      return res;
  }

  // Next player is ready to read
  if (FD_ISSET(next_player_pipe, &players->current_fds)) {
    char c_move; // Read to a char because C enums are always 4 bytes
    res = read(next_player_pipe, &c_move, 1);
    *move = c_move;
  }

  // Advance player
  players->current_player_idx =
      (players->current_player_idx + 1) % players->game_state->n_players;

  // Return the value of read, or -1 on error in either select or read
  return res;
}

int players_all_blocked(players_t players) {
  for (int i = 0; i < players->game_state->n_players; i++) {
    if (!players->game_state->players[i].blocked)
      return 0;
  }

  return 1;
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
