#ifndef GRAPHICS_H
#define GRAPHICS_H

#include "game_state.h"
#define C_GRAY 20
#define CP_GRAYSCALE 1

#define CP_PLAYER 10

#define HEADER_HEIGHT 5

#define CELL_WIDTH 5
#define CELL_HEIGHT 3
#define BOARD_OFFSET HEADER_HEIGHT

#define y1(i) ((i) * CELL_HEIGHT + BOARD_OFFSET)
#define x1(j) ((j) * CELL_WIDTH)
#define y2(i) ((i) * CELL_HEIGHT + CELL_HEIGHT - 1 + BOARD_OFFSET)
#define x2(j) ((j) * CELL_WIDTH + CELL_WIDTH - 1)

void gfx_init();

void draw_cell(int i, int j, game_state_t *game_state);

#endif
