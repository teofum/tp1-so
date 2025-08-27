#ifndef GRAPHICS_H
#define GRAPHICS_H

#include "game_state.h"
#define C_GRAY 20
#define CP_GRAYSCALE 1

#define CP_PLAYER 10

#define HEADER_HEIGHT 5
#define HEADER_MIN_WIDTH 18
#define HEADER_CAT_WIDTH 23

#define CELL_WIDTH 5
#define CELL_HEIGHT 3

void gfx_init();

void draw_grid(game_state_t *game_state);

void draw_cell(int i, int j, game_state_t *game_state);
void draw_cell_mini(int i, int j, game_state_t *game_state);

void draw_player_card(int player_idx, game_state_t *game_state);

#endif
