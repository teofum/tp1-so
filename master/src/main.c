#include "args.h"
#include <stdio.h>
#include <sys/fcntl.h>
#include <sys/mman.h>
#include <sys/stat.h>
#include <sys/wait.h>
#include <unistd.h>

void logpid() { printf("[master: %d] ", getpid()); }

int main(int argc, char **argv) {
  args_t args;
  const char *parse_err = NULL;
  if (!parse_args(argc, argv, &args, &parse_err)) {
    free_args(&args);
    printf("Failed to parse args: %s\n", parse_err);
    return -1;
  }

  logpid();
  printf("Hello world! My pid is %d\n\n", getpid());

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
  logpid();
  printf("Creating shared memory...\n");
  int shm_fd = shm_open("/game_state", O_RDWR | O_CREAT, 0);
  if (shm_fd < 0) {
    logpid();
    printf("Failed to create shared memory\n");
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
  logpid();
  printf("Spawning view process...\n");
  int view_pid = fork();
  if (view_pid == -1) {
    logpid();
    printf("Failed to fork view process\n");
    return -1;
  } else if (!view_pid) {
    close(shm_fd);
    execv(args.view, argv);
  }

  /*
   * Fork and exec player processes
   */
  logpid();
  printf("Spawning player processes...\n");
  int player_pids[MAX_PLAYERS];
  int pipes_transmit[2][MAX_PLAYERS];
  int pipes_receive[2][MAX_PLAYERS];

  for (int i = 0; args.players[i] != NULL; i++) {
    // Create pipes to communicate with player
    if (pipe(pipes_transmit[i]) == -1 || pipe(pipes_receive[i]) == -1) {
      logpid();
      printf("Failed to create pipes for player %d\n", i + 1);
      return -1;
    }

    int pid = fork();
    if (pid == -1) {
      logpid();
      printf("Failed to fork player %d process\n", i + 1);
      return -1;
    } else if (!pid) {
      logpid();
      printf("Child process %d setting up pipes\n", i + 1);
      // Connect player stdin to master -> player pipe
      dup2(pipes_transmit[i][0], 0);
      // And stdout to player -> master pipe
      dup2(pipes_receive[i][1], 1);

      // Close unused fds (player)
      close(pipes_transmit[i][1]);
      close(pipes_receive[i][0]);

      execv(args.players[i], argv);
    }

    // Close unused fds (master)
    close(pipes_transmit[i][0]);
    close(pipes_receive[i][1]);

    player_pids[i] = pid;
  }

  /// en  este punto estan los pipes en pipesr/w y los hijos de players creados

  logpid();
  printf("Player communication test...\n");
  char buf[256];
  for (int i = 0; args.players[i] != NULL; i++) {
    dprintf(pipes_transmit[i][1], "Hello player %d", i + 1);
    int read_bytes = read(pipes_receive[i][0], buf, 256);
    buf[read_bytes] = 0;

    logpid();
    printf("Player %d response: %s\n", i + 1, buf);
  }

  /*
   * Wait for child processes and clean up resources
   */
  logpid();
  printf("Waiting for child processes to end...\n");
  for (int i = 0; args.players[i] != NULL; i++) {
    int pid = player_pids[i];
    int ret;
    waitpid(pid, &ret, 0);
    logpid();
    printf("Player %d process with pid %d exited with code %d\n", i + 1, pid,
           ret);
  }

  logpid();
  printf("Waiting for view process to end...\n");
  int ret;
  waitpid(view_pid, &ret, 0);
  logpid();
  printf("View process exited with code %d\n", ret);

  logpid();
  printf("Unlinking shared memory...\n");
  shm_unlink("/game_state");

  free_args(&args);

  logpid();
  printf("Bye!\n");
  return 0;
}
