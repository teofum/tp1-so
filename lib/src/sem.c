#include <sem.h>

int semaphore_create(sem_t *sem, sem_scope_t scope, uint32_t initial_value) {
  return sem_init(sem, scope, initial_value);
}

int semaphore_destroy(sem_t *sem) { return sem_destroy(sem); }

int semaphore_wait(sem_t *sem) { return sem_wait(sem); }

int semaphore_post(sem_t *sem) { return sem_post(sem); }
