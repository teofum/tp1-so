#ifndef CALLBACK_H
#define CALLBACK_H

#include <game.h>

void player_wait_callback(player_t *player, uint32_t idx, int ret);

void view_wait_callback(pid_t pid, int ret);

#endif
