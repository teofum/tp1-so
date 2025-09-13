#ifndef TIMEOUT_H
#define TIMEOUT_H

#define sec_to_micros(x) ((x) * 1000000)

#include <stdint.h>

typedef struct timeout_cdt_t *timeout_t;
typedef struct timeval timeval_t;

/*
 * Create a new timeout. Takes the duration, in microseconds.
 */
timeout_t timeout_create(uint64_t duration);

/*
 * Check if timeout has timed out since last reset. Returns 1 if timed out,
 * 0 otherwise.
 */
int timeout_check(timeout_t timeout);

/*
 * Query remaining time on the timeout, in microseconds.
 */
uint64_t timeout_remaining(timeout_t timeout);

/*
 * Reset a timeout.
 */
void timeout_reset(timeout_t timeout);

/*
 * Destroy a timeout, freeing all resources.
 */
void timeout_destroy(timeout_t timeout);

#endif
