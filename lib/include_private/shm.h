/*
 * Wrapper API for POSIX shared memory syscalls.
 */

#ifndef SHM_H
#define SHM_H

#include <stddef.h>

typedef enum {
  SHM_READ = 0,
  SHM_READ_WRITE = 0x1,
  SHM_CREATE = 0x2,
} shm_mode_t;

/*
 * Utility shared memory wrapper to open and memory map a shared memory
 * object as one operation.
 */
void *shm_open_and_map(const char *name, shm_mode_t mode, size_t size);

/*
 * Wrapper for shm_unlink
 */
void shm_disconnect(const char *name);

#endif
