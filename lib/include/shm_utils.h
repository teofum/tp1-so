#ifndef SHM_UTILS_H
#define SHM_UTILS_H

#include <stddef.h>

void *shm_open_and_map(const char *name, int mode, size_t size);

#endif
