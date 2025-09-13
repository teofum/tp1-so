#include <utils.h>

char to_move(int dx, int dy) {
  static char moves[3][3] = {{7, 0, 1}, {6, -1, 2}, {5, 4, 3}};

  return moves[dy + 1][dx + 1];
}

int dx(int move) {
  static int x[8] = {0, 1, 1, 1, 0, -1, -1, -1};
  return x[move];
}

int dy(int move) {
  static int y[8] = {-1, -1, 0, 1, 1, 1, 0, -1};
  return y[move];
}

int check_bounds(int x, int y, game_state_t *gs) {
  return (x >= 0 && y >= 0 && x < gs->board_width && y < gs->board_height);
}

int available(int x, int y, game_state_t *gs) {
  return check_bounds(x, y, gs) && gs->board[x + y * gs->board_width] > 0;
}

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