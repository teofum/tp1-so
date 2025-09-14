#include <game_sync.h>

void game_sync_init(game_sync_t *game_sync, uint32_t n_players) {
  game_sync->read_count = 0;

  // Initialize semaphores
  semaphore_create(&game_sync->view_should_update, SEM_SCOPE_PROCS, 0);
  semaphore_create(&game_sync->view_did_update, SEM_SCOPE_PROCS, 0);

  semaphore_create(&game_sync->master_write_mutex, SEM_SCOPE_PROCS, 1);
  semaphore_create(&game_sync->game_state_mutex, SEM_SCOPE_PROCS, 1);
  semaphore_create(&game_sync->read_count_mutex, SEM_SCOPE_PROCS, 1);

  for (int i = 0; i < n_players; ++i) {
    semaphore_create(&game_sync->player_may_move[i], SEM_SCOPE_PROCS, 1);
  }
}

void game_sync_destroy(game_sync_t *game_sync, uint32_t n_players) {
  semaphore_destroy(&game_sync->view_should_update);
  semaphore_destroy(&game_sync->view_did_update);

  semaphore_destroy(&game_sync->master_write_mutex);
  semaphore_destroy(&game_sync->game_state_mutex);
  semaphore_destroy(&game_sync->read_count_mutex);

  for (int i = 0; i < n_players; ++i) {
    semaphore_destroy(&game_sync->player_may_move[i]);
  }
}
