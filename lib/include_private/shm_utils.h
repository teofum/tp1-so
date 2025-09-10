#ifndef SHM_UTILS_H
#define SHM_UTILS_H

#include <stddef.h>
#include <sys/fcntl.h>
#include <sys/mman.h>
#include <unistd.h>

/*
 * Utility shared memory wrapper to open and memory map a shared memory
 * object as one operation.
 */
void *shm_open_and_map(const char *name, int mode, size_t size);

#endif
