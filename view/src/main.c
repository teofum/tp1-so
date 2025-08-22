#include <stdio.h>
#include <sys/fcntl.h>
#include <sys/mman.h>
#include <sys/stat.h>
#include <sys/wait.h>
#include <unistd.h>

int main(int argc, char **argv) {
  // TODO parse args
  printf("[view] Hello world\n");

  /*
   * Set up shared memory
   */
  printf("[view] Opening shared memory...\n");
  int shm_fd = shm_open("/game_state", O_RDONLY, 0);
  if (shm_fd < 0) {
    printf("[view] Failed to open shared memory\n");
    return -2;
  }

  void *mem = mmap(NULL, 100, PROT_READ, MAP_SHARED, shm_fd, 0);
  // TODO null check

  printf("[view] master says: %s\n", (const char *)mem);

  return 0;
}
