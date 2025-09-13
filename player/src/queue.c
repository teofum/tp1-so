#include <queue.h>

// Initialize queue
void queue_init(queue_t *q) {
  q->front = 0;
  q->rear = 0;
  q->size = 0;
}

// Check if queue is empty
bool queue_empty(queue_t *q) { return q->size == 0; }

// Check if queue is full
bool queue_full(queue_t *q) { return q->size == QUEUE_CAPACITY; }

// Add a Pair to the queue
bool queue_enqueue(queue_t *q, int2_t value) {
  if (queue_full(q))
    return false;

  q->data[q->rear] = value;
  q->rear = (q->rear + 1) % QUEUE_CAPACITY;
  q->size++;
  return true;
}

// Remove a Pair from the queue
bool queue_dequeue(queue_t *q, int2_t *out) {
  if (queue_empty(q))
    return false;

  *out = q->data[q->front];
  q->front = (q->front + 1) % QUEUE_CAPACITY;
  q->size--;
  return true;
}
