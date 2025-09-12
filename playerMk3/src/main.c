#include <args.h>
#include <game.h>

#include <stdio.h>

void logpid() { printf("[player: %d] ", getpid()); }
void logerr(const char *s) {
  fprintf(stderr, "[player: %d] %s\n", getpid(), s);
}

// TODO move this to files etc

// Convert diff 'x and y' into char dir
// !Asume que no hay dx=dy=0 que no deberia pasar
char getDir(int dx, int dy){
  char dirs[3][3] ={{7, 0,1},
                    {6,-1,2},
                    {5, 4,3}};

  return dirs[dy + 1][dx + 1];
}

// returns 0 if out of bounds
int inBounds(int x,int y, game_state_t* gs){
  if( x < 0 || y < 0 ||
      x >= gs->board_width ||
      y >= gs->board_height ||
      ( gs->board[ x + (y * gs->board_width) ] <= 0 )
    ){
    return 0;
  }
  return 1;
}

int checkBox( int x, int y, game_state_t* game_state){
  int sum = 0;
  for(int dy=-1; dy<=1; ++dy){
    for(int dx=-1; dx<=1; ++dx){
      if( inBounds( x + dx, y + dy, game_state ) ){
        int kernelIndex = ((x + dx)+((y + dy) * game_state->board_width ));
        int aux = game_state->board[kernelIndex];
        sum += (aux * aux);
      }
    }
  }
  return sum;
}

// Generate next move :T
char get_next_move(game_state_t* game_state, int player_idx) {
  char next=-1;
  int max=-1;

  int x = game_state->players[player_idx].x;
  int y = game_state->players[player_idx].y; 

  for(int dy=-1; dy<=1; ++dy){
    for(int dx=-1; dx<=1; ++dx){
      if( !(dx==0 && dy==0) && inBounds( x + dx, y + dy, game_state) ){
        int cuadVal = checkBox(x + dx ,y + dy ,game_state);
        if( cuadVal > max ){
          next=getDir(dx,dy);
          max=cuadVal;
        }
      }
    }
  }

  return next;
}

int main(int argc, char **argv) {
  /*
   * Parse command line args and calculate size of game state
   */
  if (argc < 3) {
    return -1;
  }
  int width = atoi(argv[1]);
  int height = atoi(argv[2]);

  game_t game = game_connect(width, height);
  if (!game) {
    logerr("Failed to connect to game\n");
    return -1;
  }

  // Pointer to game state for convenience
  // This is the actual shared state, be careful when reading it!
  game_state_t *state = game_state(game);

  int player_idx = -1;
  int pid = getpid();
  for (int i = 0; i < state->n_players && player_idx == -1; i++) {
    if (state->players[i].pid == pid)
      player_idx = i;
  }

  /*
   * Main loop
   */
  int running = 1;
  while (running) {
    game_wait_move_processed(game, player_idx);
    game_will_read_state(game);

    // If the game ended or we're blocked, stop
    if (state->game_ended || state->players[player_idx].blocked) {
      running = 0;
    }

    game_did_read_state(game);

    if (!running)
      break;

    char next_move = get_next_move(state,player_idx);

    // Send next move to master
    write(STDOUT_FILENO, &next_move, 1);
  }

  game_disconnect(game);

  return 0;
}