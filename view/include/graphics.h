#ifndef GRAPHICS_H
#define GRAPHICS_H

#include <game_state.h>

void gfx_init();

void draw_grid(game_state_t *game_state);

void draw_cell(int i, int j, game_state_t *game_state);
void draw_cell_mini(int i, int j, game_state_t *game_state);

void draw_player_card(int player_idx, game_state_t *game_state);

void draw_game_over(game_state_t *game_state);

#endif
