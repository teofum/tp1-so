#include <args.h>
#include <game.h>
#include <move.h>
#include <utils.h>

#include <stdio.h>
#include <stdlib.h>

void logerr(const char *s) {
  fprintf(stderr, "[player: %d] %s\n", getpid(), s);
}

game_t game = NULL;

void cleanup() {
  if (game)
    game_disconnect(game);
}
#include <stdlib.h>
#include <string.h>

#include <stdlib.h>
#include <string.h>

static inline int inside_rect(int x, int y, int w, int h) {
    return (x >= 0 && y >= 0 && x < w && y < h);
}


int main(int argc, char **argv) {
  /*
   * Parse command line args and calculate size of game state
   */
  if (argc < 3) {
    return -1;
  }
  int width = atoi(argv[1]);
  int height = atoi(argv[2]);

  game = game_connect(width, height);
  if (!game) {
    logerr("Failed to connect to game\n");
    return -1;
  }
  atexit(cleanup);

  // Pointer to game state for convenience
  // This is the actual shared state, be careful when reading it!
  game_state_t *state = game_state(game);

  int player_idx = -1;
  int pid = getpid();
  for (int i = 0; i < state->n_players && player_idx == -1; i++) {
    if (state->players[i].pid == pid)
      player_idx = i;
  }

  // player flags n stuff / para el WallHug
  char next_move = 0;
  game_state_t *local_state = malloc(game_state_size(game));

  srand(player_idx);

  /*
   * Main loop
   */
  int running = 1;
  while (running) {
    game_wait_move_processed(game, player_idx);
    game_will_read_state(game);

    game_clone_state(game, local_state);

    // If the game ended or we're blocked, stop
    if (state->game_ended || state->players[player_idx].blocked) {
      running = 0;
    }

    game_did_read_state(game);

    if (!running)
      break;

    next_move = get_next_move(local_state, player_idx, next_move);
    if (next_move < 0)
      break;

    // Send next move to master
    write(STDOUT_FILENO, &next_move, 1);
  }

  return 0;
}
