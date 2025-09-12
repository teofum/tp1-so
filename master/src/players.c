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
static int players_select(players_t players, timeout_t timeout) {
  uint32_t n_players = players->game_state->n_players;

  // Fill fdset with fds of all players' pipes
  for (int i = 0; i < n_players; i++) {
    FD_SET(players->pipe_fds[i], &players->current_fds);
  }

  // Get timeout as timeval struct
  // This makes the select call wait for as long as the timeout has time
  // remaining. If it times out, it will return 0, causing players_next to
  // return will_move=0 and the game to immediately time out.
  uint64_t timeout_micros = timeout_remaining(timeout);
  timeval_t timeout_tv = {.tv_sec = timeout_micros / 1000000,
                          .tv_usec = timeout_micros % 1000000};

  // Do select syscall to get ready players
  int max_pipe = players->pipe_fds[n_players - 1];
  return select(max_pipe + 1, &players->current_fds, NULL, NULL, &timeout_tv);
}

player_move_t players_next(players_t players, timeout_t timeout) {
  int next_player = players->current_player_idx;
  int next_player_pipe = players->pipe_fds[next_player];

  int res;
  char move; // Read to a char because C enums are always 4 bytes

  // If the fdset is empty we need to call select
  // On error, return error and let the caller handle it
  if (players_fdset_empty(players)) {
    res = players_select(players, timeout);
    if (res < 0)
      return (player_move_t){.error = 1};
  }

  // Next player is ready to read
  if (FD_ISSET(next_player_pipe, &players->current_fds)) {
    FD_CLR(next_player_pipe, &players->current_fds);
    res = read(next_player_pipe, &move, 1);
    if (res < 0)
      return (player_move_t){.error = 1};
  }

  // Advance player
  players->current_player_idx =
      (players->current_player_idx + 1) % players->game_state->n_players;

  return (player_move_t){.error = 0,
                         .will_move = res > 0,
                         .player_idx = next_player,
                         .move = move};
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
      callback(player, i, ret);
  }

  free(players);
}
