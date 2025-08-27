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
#define BOARD_OFFSET HEADER_HEIGHT

#define y1(i, o) ((i) * CELL_HEIGHT + HEADER_HEIGHT * o)
#define x1(j) ((j) * CELL_WIDTH)
#define y2(i, o) ((i) * CELL_HEIGHT + CELL_HEIGHT - 1 + HEADER_HEIGHT * o)
#define x2(j) ((j) * CELL_WIDTH + CELL_WIDTH - 1)

void gfx_init();

void draw_cell(int i, int j, game_state_t *game_state);

void draw_player_card(int player_idx, game_state_t *game_state);

#endif
