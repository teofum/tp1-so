#include <args.h>
#include <game.h>
#include <move.h>
#include <utils.h>

#include <stdio.h>

//#include "queue.c"///////////////////////////////////////////////////
#include <stdio.h>
#include <stdbool.h>

#define QUEUE_CAPACITY 1000

// Define a pair of integers (x, y)
typedef struct {
    int x;
    int y;
} Pair;

// Queue of Pairs
typedef struct {
    Pair data[QUEUE_CAPACITY];
    int front;
    int rear;
    int size;
} Queue;

// Initialize queue
void initQueue4(Queue* q) {
    q->front = 0;
    q->rear = 0;
    q->size = 0;
}

// Check if queue is empty
bool isEmpty4(Queue* q) {
    return q->size == 0;
}

// Check if queue is full
bool isFull4(Queue* q) {
    return q->size == QUEUE_CAPACITY;
}

// Add a Pair to the queue
bool enqueue4(Queue* q, Pair value) {
    if (isFull4(q)) return false;
    q->data[q->rear] = value;
    q->rear = (q->rear + 1) % QUEUE_CAPACITY;
    q->size++;
    return true;
}

// Remove a Pair from the queue
bool dequeue4(Queue* q, Pair* out) {
    if (isEmpty4(q)) return false;
    *out = q->data[q->front];
    q->front = (q->front + 1) % QUEUE_CAPACITY;
    q->size--;
    return true;
}
///////////////////////////////////////////////////////////////////////
#include <stdlib.h>

void logerr(const char *s) {
  fprintf(stderr, "[player: %d] %s\n", getpid(), s);
}

game_t game = NULL;

void cleanup() {
  if (game)
    game_disconnect(game);
}
#include <stdlib.h>
#include <string.h>

#include <stdlib.h>
#include <string.h>

static inline int inside_rect(int x, int y, int w, int h) {
    return (x >= 0 && y >= 0 && x < w && y < h);
}

/**
 * BFS from (x, y).
 * - If any OTHER player is found on a checked tile => return 0.
 * - Otherwise return the number of passable (>0) tiles visited (area).
 */
int bfs_area_or_player(int x, int y, game_state_t *game_state, int player_idx){ 

  Pair start = { .x = x , .y =y };

  int size = game_state->board_width* game_state->board_height;
  int* visited = (int*)calloc(size, sizeof(int)); // TODO: esto podria estar en un mejor lugar
  if (!visited) {
    return -1;// Handle allocation failure
  }

  int area=0;

  Queue q; initQueue4(&q);
  enqueue4(&q, start);  // Add start
  Pair pivot;
  while (dequeue4(&q, &pivot)) { // dequeue4 to set the pivot
    
    for(char angle=0 ; angle < 8 ; ++angle){ // look around the pivot and enqueue4 valid
      Pair curr = {.x = pivot.x + getX(angle), .y =  pivot.y + getY(angle)};
      int currIndex = curr.x + (curr.y * game_state->board_width);
      
      for(int i=0; i<game_state->n_players; i++){ //check if player is on it
        if( i!=player_idx && game_state->players[i].x == curr.x && game_state->players[i].y == curr.y){
          return 0;
        }
      }
      if(inBounds( curr.x, curr.y, game_state) && !visited[currIndex]){
        enqueue4(&q, curr);
        visited[currIndex]=1;
        ++area;
      }
    }
  
  }

  return area;  // 0 means either found a player OR no passable tiles reachable
}

char get_next_move_Mk4(game_state_t* game_state, int player_idx, char prev){
  
  int x = game_state->players[player_idx].x;
  int y = game_state->players[player_idx].y;
    
  if(bfs_area_or_player( x, y, game_state, player_idx)){
    // esta solo
    return get_next_move_WallHug_L(game_state, player_idx, prev);
  }else{
    //si hay un playe
    //return get_next_move_Mk3(game_state, player_idx);
    return get_next_move_Mk2(game_state, player_idx);
  }
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

  game = game_connect(width, height);
  if (!game) {
    logerr("Failed to connect to game\n");
    return -1;
  }
  atexit(cleanup);

  // Pointer to game state for convenience
  // This is the actual shared state, be careful when reading it!
  game_state_t *state = game_state(game);

  int player_idx = -1;
  int pid = getpid();
  for (int i = 0; i < state->n_players && player_idx == -1; i++) {
    if (state->players[i].pid == pid)
      player_idx = i;
  }

  // player flags n stuff / para el WallHug
  char next_move = 0;
  game_state_t local_state;

  srand(player_idx);

  /*
   * Main loop
   */
  int running = 1;
  while (running) {
    game_wait_move_processed(game, player_idx);
    game_will_read_state(game);

    local_state = game_clone_state(game);

    // If the game ended or we're blocked, stop
    if (state->game_ended || state->players[player_idx].blocked) {
      running = 0;
    }

    game_did_read_state(game);

    if (!running)
      break;

    next_move = get_next_move_Mk4(state,player_idx, next_move);

    // Send next move to master
    write(STDOUT_FILENO, &next_move, 1);
  }

  return 0;
}
