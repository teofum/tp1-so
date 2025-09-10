/*
 * Shared memory wrapper implementation.
 * Replace this to use some other shared memory API.
 */

#include <shm.h>

#include <sys/fcntl.h>
#include <sys/mman.h>
#include <unistd.h>

void *shm_open_and_map(const char *name, shm_mode_t mode, size_t size) {
  int shm_mode = O_RDONLY;
  if (mode & SHM_READ_WRITE)
    shm_mode |= O_RDWR;
  if (mode & SHM_CREATE)
    shm_mode |= (O_CREAT | O_EXCL);

  int fd = shm_open(name, shm_mode, 0);
  if (fd < 0)
    return NULL;

  if (mode & SHM_CREATE)
    ftruncate(fd, size);

  int mmap_mode = PROT_READ;
  if (mode & SHM_READ_WRITE)
    mmap_mode |= PROT_WRITE;

  void *mem = mmap(NULL, size, mmap_mode, MAP_SHARED, fd, 0);

  close(fd);

  return mem;
}

void shm_disconnect(const char *name) { shm_unlink(name); }
