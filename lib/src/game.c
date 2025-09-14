#include <game.h>
#include <game_state_impl.h>
#include <game_sync.h>
#include <sem.h>
#include <shm.h>

#include <stdlib.h>
#include <string.h>

/*
 * CDT for game ADT
 */
struct game_cdt_t {
  game_state_t *state;
  game_sync_t *sync;

  size_t state_size;
};

/*
 * Initialize game, used by master.
 */
game_t game_init(args_t *args, const char **err) {
  game_t game = malloc(sizeof(struct game_cdt_t));
  if (!game) {
    if (err)
      *err = "Allocation failed";
    return NULL;
  }

  // Calculate game state struct size
  game->state_size = get_game_state_size(args->width, args->height);

  // Create and map shared memory
  game->state = shm_open_and_map("/game_state", SHM_READ_WRITE | SHM_CREATE,
                                 game->state_size);
  if (!game->state) {
    if (err)
      *err = "Failed to map shared memory /game_state";
    free(game);
    return NULL;
  }

  game->sync = shm_open_and_map("/game_sync", SHM_READ_WRITE | SHM_CREATE,
                                sizeof(game_sync_t));
  if (!game->sync) {
    if (err)
      *err = "Failed to map shared memory /game_sync";
    free(game);
    return NULL;
  }

  // Initialize game
  game_state_init(game->state, args);
  game_sync_init(game->sync, game->state->n_players);

  return game;
}

/*
 * Connect to existing game, used by view/players
 */
game_t game_connect(uint32_t width, uint32_t height) {
  game_t game = malloc(sizeof(struct game_cdt_t));
  if (!game)
    return NULL;

  game->state_size = get_game_state_size(width, height);
  game->state = shm_open_and_map("/game_state", SHM_READ, game->state_size);
  if (!game->state) {
    free(game);
    return NULL;
  }

  game->sync =
      shm_open_and_map("/game_sync", SHM_READ_WRITE, sizeof(game_sync_t));
  if (!game->sync) {
    free(game);
    return NULL;
  }

  return game;
}

/*
 * Disconnect from a game, used by view/players
 */
void game_disconnect(game_t game) {
  shm_disconnect("/game_state");
  shm_disconnect("/game_sync");

  game->state = NULL;
  game->sync = NULL;

  free(game);
}

/*
 * Disconnect from a game and destroy it, used by master
 * This should be called last, obviously
 */
void game_destroy(game_t game) {
  game_sync_destroy(game->sync, game->state->n_players);
  game_disconnect(game);
}

void game_end(game_t game) {
  // Release all locks to allow processes to exit
  semaphore_post(&game->sync->view_should_update);
  for (int i = 0; i < game->state->n_players; i++)
    semaphore_post(&game->sync->player_may_move[i]);

  game->state->game_ended = 1;
}

// === State access ===========================================================

game_state_t *game_state(game_t game) { return game->state; }

size_t game_state_size(game_t game) { return game->state_size; }

void game_clone_state(game_t game, game_state_t *dst) {
  memcpy(dst, game->state, game->state_size);
}

// === Sync functions =========================================================

void game_will_read_state(game_t game) {
  // Wait for writer
  semaphore_wait(&game->sync->master_write_mutex);
  semaphore_post(&game->sync->master_write_mutex);

  // Lightswitch sync
  semaphore_wait(&game->sync->read_count_mutex);
  if (game->sync->read_count++ == 0) {
    semaphore_wait(&game->sync->game_state_mutex);
  }
  semaphore_post(&game->sync->read_count_mutex);
}

void game_did_read_state(game_t game) {
  semaphore_wait(&game->sync->read_count_mutex);
  if (--game->sync->read_count == 0) {
    semaphore_post(&game->sync->game_state_mutex);
  }
  semaphore_post(&game->sync->read_count_mutex);
}

void game_lock_state_for_writing(game_t game) {
  semaphore_wait(&game->sync->master_write_mutex);
  semaphore_wait(&game->sync->game_state_mutex);
  semaphore_post(&game->sync->master_write_mutex);
}

void game_release_state(game_t game) {
  semaphore_post(&game->sync->game_state_mutex);
}

void game_wait_move_processed(game_t game, size_t player_idx) {
  semaphore_wait(&game->sync->player_may_move[player_idx]);
}

void game_post_move_processed(game_t game, size_t player_idx) {
  semaphore_post(&game->sync->player_may_move[player_idx]);
}

void game_update_view(game_t game) {
  semaphore_post(&game->sync->view_should_update);
  semaphore_wait(&game->sync->view_did_update);
}

void game_wait_view_should_update(game_t game) {
  semaphore_wait(&game->sync->view_should_update);
}

void game_post_view_did_update(game_t game) {
  semaphore_post(&game->sync->view_did_update);
}
