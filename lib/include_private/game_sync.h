/*
 * Game synchronization primitives.
 * Private implementation detail not exposed to the caller. All synchronization
 * is handled through the game ADT.
 */

#ifndef GAME_SYNC_H
#define GAME_SYNC_H

#include <sem.h>
#include <stdint.h>

typedef struct {
  sem_t view_should_update;
  sem_t view_did_update;

  sem_t master_write_mutex;
  sem_t game_state_mutex;
  sem_t read_count_mutex;
  uint32_t read_count;

  sem_t player_may_move[9];
} game_sync_t;

/*
 * Initialize semaphores
 */
void game_sync_init(game_sync_t *game_sync, uint32_t n_players);

/*
 * Destroy semaphores
 */
void game_sync_destroy(game_sync_t *game_sync, uint32_t n_players);

#endif
