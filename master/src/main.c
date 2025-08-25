#include <args.h>
#include <game_state.h>
#include <game_sync.h>
#include <shm_utils.h>
#include <spawn.h>

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

  game_state_t *game_state =
      shm_open_and_map("/game_state", O_RDWR | O_CREAT, sizeof(game_state_t));
  if (!game_state) {
    logpid();
    printf("Failed to create shared memory game_state\n");
    return -1;
  }

  game_sync_t *game_sync =
      shm_open_and_map("/game_sync", O_RDWR | O_CREAT, sizeof(game_sync_t));
  if (!game_state) {
    logpid();
    printf("Failed to create shared memory game_sync\n");
    return -1;
  }

  // test
  game_state->n_players = 1337;

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
    execv(args.view, argv);
  }

  /*
   * Fork and exec player processes
   */
  logpid();
  printf("Spawning player processes...\n");
  player_data_t players[MAX_PLAYERS];

  for (int i = 0; args.players[i] != NULL; i++) {
    players[i] = spawn_player(args.players[i], argv);
  }

  logpid();
  printf("Player communication test...\n");
  char buf[256];
  for (int i = 0; args.players[i] != NULL; i++) {
    dprintf(players[i].pipe_tx, "Hello player %d", i + 1);
    int read_bytes = read(players[i].pipe_rx, buf, 256);
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
    int pid = players[i].pid;
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
