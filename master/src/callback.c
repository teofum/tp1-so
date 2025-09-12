#include <callback.h>

#include <stdio.h>

void player_wait_callback(player_t *player, uint32_t idx, int ret) {
  printf("Player %u (%s, pid: %d) exited with code %d\n", idx, player->name,
         player->pid, ret);
  printf("  Score: %4u, Valid: %4u, Invalid: %4u\n", player->score,
         player->requests_valid, player->requests_invalid);
}

void view_wait_callback(pid_t pid, int ret) {
  printf("View (pid: %d) exited with code %d\n", pid, ret);
}
