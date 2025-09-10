#include "game_sync.h"

void game_sync_init(game_sync_t *game_sync, uint32_t n_players) {
  game_sync->read_count = 0;

  // Initialize semaphores
  sem_init(&game_sync->view_should_update, 1, 0); // 0: view waits for master
  sem_init(&game_sync->view_did_update, 1, 0);    // 0: master waits for view

  sem_init(&game_sync->master_write_mutex, 1, 1); // 1: unlocked
  sem_init(&game_sync->game_state_mutex, 1, 1);   // 1: unlocked
  sem_init(&game_sync->read_count_mutex, 1, 1);   // 1: unlocked

  for (int i = 0; i < n_players; ++i) {
    sem_init(&game_sync->player_may_move[i], 1, 1);
  }
}

void game_sync_free(game_sync_t *game_sync, uint32_t n_players) {
  sem_destroy(&game_sync->view_should_update);
  sem_destroy(&game_sync->view_did_update);

  sem_destroy(&game_sync->master_write_mutex);
  sem_destroy(&game_sync->game_state_mutex);
  sem_destroy(&game_sync->read_count_mutex);

  for (int i = 0; i < n_players; ++i) {
    sem_destroy(&game_sync->player_may_move[i]);
  }
}
