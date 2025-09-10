#include <stdlib.h>
#include <time.h>
#include <timeout.h>

struct timeout_cdt_t {
  time_t start;
  uint64_t duration;
};

timeout_t timeout_create(uint64_t duration) {
  timeout_t timeout = malloc(sizeof(struct timeout_cdt_t));
  if (!timeout)
    return NULL;

  timeout->start = time(NULL);
  timeout->duration = duration;

  return timeout;
}

int timeout_check(timeout_t timeout) {
  time_t now = time(NULL);
  time_t elapsed = now - timeout->start;

  return elapsed >= timeout->duration;
}

void timeout_reset(timeout_t timeout) { timeout->start = time(NULL); }

void timeout_destroy(timeout_t timeout) { free(timeout); }
