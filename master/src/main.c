#include "args.h"
#include <stdio.h>
#include <sys/fcntl.h>
#include <sys/mman.h>
#include <sys/stat.h>
#include <sys/wait.h>
#include <unistd.h>

int main(int argc, char **argv) {
  args_t args;
  const char *parse_err = NULL;
  if (!parse_args(argc, argv, &args, &parse_err)) {
    free_args(&args);
    printf("Failed to parse args: %s\n", parse_err);
    return -1;
  }

  printf("[master] Hello world\n\n");

  printf("Board size %ux%u\n", args.width, args.height);
  printf("Delay %ums\n", args.delay);
  printf("Timeout %ums\n", args.timeout);
  printf("Seed %d\n", args.seed);

  if (args.view)
    printf("View executable %s\n", args.view);

  for (int i = 0; args.players[i] != NULL; i++)
    printf("Player %d: %s\n", i + 1, args.players[i]);

  /*
   * Set up shared memory
   */
  printf("[master] Creating shared memory...\n");
  int shm_fd = shm_open("/game_state", O_RDWR | O_CREAT | O_EXCL, 0);
  if (shm_fd < 0) {
    printf("[master] Failed to create shared memory\n");
    return -2;
  }

  // make it 100 bytes to test
  ftruncate(shm_fd, 100);

  void *mem = mmap(NULL, 100, PROT_READ | PROT_WRITE, MAP_SHARED, shm_fd, 0);
  // TODO null check

  sprintf(mem, "pinga");

  /*
   * Fork and exec the view process
   */
  int child_pid = fork();
  if (!child_pid) {
    close(shm_fd);

    execv(args.view, argv);
  } else {
    int ret;
    waitpid(child_pid, &ret, 0);

    printf("[master] View process exited with code %d\n", ret);
  }

  printf("[master] Unlinking shared memory...\n");
  shm_unlink("/game_state");

  free_args(&args);

  printf("Bye!\n");
  return 0;
}
