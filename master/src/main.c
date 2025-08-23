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
  int view_pid = fork();
  if (!view_pid) {
    close(shm_fd);
    execv(args.view, argv);
  }

  ///// pipe setup mk1 and fork and exec players

  int pipesw[2][MAX_PLAYERS]; // un array de pipes de write
  int pipesr[2][MAX_PLAYERS]; // un array de pipes de read

  for (int i = 0; args.players[i] != NULL; i++) {
    if (pipe(pipesw[i]) == -1 || pipe(pipesr[i]) == -1) {
      return -1; // falla el creado del pipe
    }
  }

  printf("[master] Creating player processes...\n");
  // a cada player se le manda sus respectivos pipes de read y write
  int player_pids[MAX_PLAYERS];

  for (int i = 0; args.players[i] != NULL; i++) {
    int pid = fork();
    if (pid == -1) {
      return -1;
    } // falla el fork

    if (!pid) { /// child proces
      execv(args.players[i], argv);
    } else {
      player_pids[i] = pid; // parent guarda el pid de los nenes
    }
  }

  /// en  este punto estan los pipes en pipesr/w y los hijos de players creados

  printf("[master] Waiting for child processes to end...\n");
  for (int i = 0; args.players[i] != NULL; i++) {
    int pid = player_pids[i];
    int ret;
    waitpid(pid, &ret, 0);
    printf("[master] Player %d process with pid %d exited with code %d\n",
           i + 1, pid, ret);
  }

  printf("[master] Waiting for view process to end...\n");
  int ret;
  waitpid(view_pid, &ret, 0);
  printf("[master] View process exited with code %d\n", ret);

  printf("[master] Unlinking shared memory...\n");
  shm_unlink("/game_state");

  free_args(&args);

  printf("Bye!\n");
  return 0;
}
