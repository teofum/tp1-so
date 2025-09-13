#include <stdlib.h>
#include <sys/time.h>
#include <timeout.h>

struct timeout_cdt_t {
  uint64_t start;
  uint64_t duration;
};

/*
 * Utility function, return current time in microseconds
 */
static uint64_t now() {
  timeval_t tv;
  gettimeofday(&tv, NULL);

  return sec_to_micros(tv.tv_sec) + tv.tv_usec;
}

timeout_t timeout_create(uint64_t duration) {
  timeout_t timeout = malloc(sizeof(struct timeout_cdt_t));
  if (!timeout)
    return NULL;

  timeout->start = now();
  timeout->duration = duration;

  return timeout;
}

int timeout_check(timeout_t timeout) {
  uint64_t elapsed = now() - timeout->start;
  return elapsed >= timeout->duration;
}

uint64_t timeout_remaining(timeout_t timeout) {
  uint64_t elapsed = now() - timeout->start;
  return timeout->duration - elapsed;
}

void timeout_reset(timeout_t timeout) { timeout->start = now(); }

void timeout_destroy(timeout_t timeout) { free(timeout); }
