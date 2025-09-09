#ifndef GAME_H
#define GAME_H

#include "args.h"
#include "game_state.h"
#include "game_sync.h"

typedef struct {
  game_state_t *state;
  game_sync_t *sync;
} game_t;

int game_init(game_t *game, args_t *args);

int game_connect(game_t *game, uint32_t width, uint32_t height);

void game_disconnect(game_t *game);

void game_destroy(game_t *game);

void lock_state_for_reading(game_sync_t *game_sync);

void lock_state_for_writing(game_sync_t *game_sync);

void release_state(game_sync_t *game_sync);

#endif
