#ifndef GRAPHICS_H
#define GRAPHICS_H

#include "game_state.h"
#define C_GRAY 20
#define CP_GRAYSCALE 1

#define CP_PLAYER 10

void gfx_init();

void get_cell_contents(char *buf, int value, int i, int j,
                       game_state_t *game_state);

#endif
