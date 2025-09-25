#include <stdbool.h>
#include <stdio.h>

#define QUEUE_CAPACITY 1000

// Define a pair of integers (x, y)
typedef struct {
  int x;
  int y;
} int2_t;

// Queue of Pairs
typedef struct {
  int2_t data[QUEUE_CAPACITY];
  int front;
  int rear;
  int size;
} queue_t;

// Initialize queue
void queue_init(queue_t *q);

// Check if queue is empty
bool queue_empty(queue_t *q);

// Check if queue is full
bool queue_full(queue_t *q);

// Add a Pair to the queue
bool queue_enqueue(queue_t *q, int2_t value);

// Remove a Pair from the queue
bool queue_dequeue(queue_t *q, int2_t *out);