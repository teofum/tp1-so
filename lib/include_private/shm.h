#ifndef SHM_H
#define SHM_H

#include <stddef.h>
#include <sys/fcntl.h>

typedef enum {
  SHM_READ = O_RDONLY,
  SHM_READ_WRITE = O_RDWR,
  SHM_CREATE = O_CREAT | O_EXCL,
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
