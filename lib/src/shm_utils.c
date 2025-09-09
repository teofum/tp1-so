#include <shm_utils.h>

void *shm_open_and_map(const char *name, int mode, size_t size) {
  int fd = shm_open(name, mode, 0);
  if (fd < 0)
    return NULL;

  if (mode & O_CREAT)
    ftruncate(fd, size);

  int mmap_mode = PROT_READ;
  if (mode & O_RDWR)
    mmap_mode |= PROT_WRITE;

  void *mem = mmap(NULL, size, mmap_mode, MAP_SHARED, fd, 0);

  close(fd);

  return mem;
}
