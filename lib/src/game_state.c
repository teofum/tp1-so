#include <game_state.h>

size_t get_game_state_size(uint16_t board_width, uint16_t board_height) {
  return sizeof(game_state_t) + board_width * board_height * sizeof(int32_t);
}
