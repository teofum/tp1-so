#ifndef TIMEOUT_H
#define TIMEOUT_H

#include <stdint.h>

typedef struct timeout_cdt_t *timeout_t;

/*
 * Create a new timeout. Takes the duration, in seconds.
 */
timeout_t timeout_create(uint64_t duration);

/*
 * Check if timeout has timed out since last reset. Returns 1 if timed out,
 * 0 otherwise.
 */
int timeout_check(timeout_t timeout);

/*
 * Reset a timeout.
 */
void timeout_reset(timeout_t timeout);

/*
 * Destroy a timeout, freeing all resources.
 */
void timeout_destroy(timeout_t timeout);

#endif
