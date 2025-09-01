#include <game_state.h>
#include <stdint.h>
#include <stdio.h>

size_t get_game_state_size(uint16_t board_width, uint16_t board_height) {
  return sizeof(game_state_t) + board_width * board_height * sizeof(int32_t);
}

void game_state_init(game_state_t *state, const args_t *args){
  state->board_width = args->width;
  state->board_height = args->height;
  state->game_ended = 0;

  // Count players
  uint32_t n_players = 0;
  for(uint32_t i = 0; args->players[i] != NULL && i < MAX_PLAYERS; i++){
    n_players++;
  }

  if (n_players == 0){
    fprintf(stderr, "No players received");
    return;
  }

  state->n_players = n_players;

  // Calculate board size
  size_t board_size = state->board_width * state->board_height;

  // Initialize board with rewards from 1 to 9
  for (size_t i = 0; i < board_size; i++) {
    state->board[i] = (rand() % 9) + 1;
  }

  // Initialize players
  for(uint32_t i = 0; i < n_players; i++){
    strncpy(state->players[i].name, args->players[i], MAX_PLAYER_NAME - 1);
    state->players[i].name[MAX_PLAYER_NAME - 1] = '\0';
    state->players[i].score = 0;
    state->players[i].requests_invalid = 0;
    state->players[i].requests_valid = 0;
    state->players[i].blocked = 0;
    // state->players[i].pid = 0; Set by master, need to be initialize?

    // Divide board in (n_players+1) vertical cols
    // (n_players+1) to generate 'margins' and avoid players in border
    uint16_t col = ((i + 1) * state->board_width)  / (n_players + 1);

    // Divide board in (2*n_players) horizontal rows
    // Pick (i*2+1) rows to leave a row empty between players
    uint16_t row = ((i * 2 + 1) * state->board_height) / (2 * n_players);

    state->players[i].x = col;
    state->players[i].y = row;

    // Mark player position on the board
    int pos = state->players[i].y * state->board_width + state->players[i].x;
    state->board[pos] = -(i + 1);
  }
}

