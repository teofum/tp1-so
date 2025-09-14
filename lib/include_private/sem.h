/*
 * Simple wrapper around the system semaphore APIs
 * Intended to be easily replaceable on a different system
 */

#ifndef SEM_H
#define SEM_H

#include <semaphore.h>
#include <stdint.h>

// This typedef is obviously redundant, but the idea is you could eventually
// change it on a system with a different named semaphore type
typedef sem_t sem_t;

typedef enum {
  SEM_SCOPE_THREADS = 0, // Shared between threads of the same process
  SEM_SCOPE_PROCS = 1,   // Shared between processes
} sem_scope_t;

// semaphore_ prefix is a little verbose but sem_ is taken for a lot of function
// names

/*
 * Create a semaphore
 */
int semaphore_create(sem_t *sem, sem_scope_t scope, uint32_t initial_value);

/*
 * Destroy a semaphore
 */
int semaphore_destroy(sem_t *sem);

// i'd rather have the sem_ prefix but those names are taken, sadge

/*
 * If the semaphore value is positive, decrease it and continue.
 * If it is zero, blocks until there is a positive value.
 */
int semaphore_wait(sem_t *sem);

/*
 * If the semaphore value is positive, decrease it and continue.
 * If it is zero, blocks until there is a positive value or time runs out.
 * Returns an error if timed out.
 */
int semaphore_timed_wait(sem_t *sem, uint64_t timeout);

/*
 * Increment the semaphore value by 1.
 */
int semaphore_post(sem_t *sem);

#endif
