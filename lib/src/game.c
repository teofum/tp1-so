#include "game.h"
#include "game_sync.h"
#include "shm_utils.h"

int game_init(game_t *game, args_t *args) {
  /*
   * Set up shared memory
   */
  game->state =
      shm_open_and_map("/game_state", O_RDWR | O_CREAT,
                       get_game_state_size(args->width, args->height));
  if (!game->state)
    return 0;

  game->sync =
      shm_open_and_map("/game_sync", O_RDWR | O_CREAT, sizeof(game_sync_t));
  if (!game->sync)
    return 0;

  game_state_init(game->state, args);
  game_sync_init(game->sync, game->state->n_players);

  return 1;
}

int game_connect(game_t *game, uint32_t width, uint32_t height) {
  game->state = shm_open_and_map("/game_state", O_RDONLY,
                                 get_game_state_size(width, height));
  if (!game->state)
    return 0;

  game->sync = shm_open_and_map("/game_sync", O_RDWR, sizeof(game_sync_t));
  if (!game->sync)
    return 0;

  return 1;
}

void game_disconnect(game_t *game) {
  shm_unlink("/game_state");
  shm_unlink("/game_sync");

  game->state = NULL;
  game->sync = NULL;
}

void game_destroy(game_t *game) {
  game_sync_free(game->sync, game->state->n_players);
}
