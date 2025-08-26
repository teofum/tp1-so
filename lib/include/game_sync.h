#ifndef GAME_SYNC_H
#define GAME_SYNC_H

#include <semaphore.h>
#include <stdint.h>

typedef struct {
  sem_t view_should_update;
  sem_t view_did_update;
  sem_t game_sync_lock;
  sem_t game_state_lock;
  sem_t active_player_count_lock;
  uint32_t active_player_count;
  sem_t player_may_move[9];
} game_sync_t;

#endif
