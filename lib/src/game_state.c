#include <game_state.h>

size_t get_game_state_size(uint16_t board_width, uint16_t board_height) {
  return sizeof(game_state_t) + board_width * board_height * sizeof(int32_t);
}

/*
 * Aplica el move, retorna 0 si fue invalido y 1 si se aplico
 */
int make_move(int player, char dir, game_state_t* game_state){
  int x =game_state->players[player].x;
  int y = game_state->players[player].y;

  int mx, my;

  if(0 < dir < 4){
    ++mx;
  }else if(4 < dir){
    --mx;
  }
  if(dir < 2 || dir > 6){
    ++my;
  }else if(2 < dir < 6){
    --my;
  }

  int curpos = ( game_state->board_width * y + x );
  int newpos = (curpos +( game_state->board_width * my + mx ));

  //check if valid
  if( !(0<=(x+mx)<game_state->board_width) || !(0<=(y+my)<game_state->board_height) || game_state->board[newpos]<=0){
    ++game_state->players[player].requests_invalid;
    return 0;
  }

  ++game_state->players[player].requests_valid;

  game_state->players[player].score += game_state->board[newpos];
  game_state->board[newpos] = -player;  

  game_state->players[player].x= x + mx;
  game_state->players[player].y= y + my;
  
  return 1; 
}

// pido perdon necesito poner a andar un formater