#ifndef GAME_SYNC_H
#define GAME_SYNC_H

#include <semaphore.h>
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

#endif
