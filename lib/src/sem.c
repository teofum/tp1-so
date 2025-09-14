#include <sem.h>
#include <time.h>

int semaphore_create(sem_t *sem, sem_scope_t scope, uint32_t initial_value) {
  return sem_init(sem, scope, initial_value);
}

int semaphore_destroy(sem_t *sem) { return sem_destroy(sem); }

int semaphore_wait(sem_t *sem) { return sem_wait(sem); }

int semaphore_timed_wait(sem_t *sem, uint64_t timeout) {
  struct timespec timeout_ts;
  timespec_get(&timeout_ts, TIME_UTC);

  timeout_ts.tv_sec += timeout / 1000000;
  timeout_ts.tv_nsec += (timeout % 1000000) * 1000;

  return sem_timedwait(sem, &timeout_ts);
}

int semaphore_post(sem_t *sem) { return sem_post(sem); }
