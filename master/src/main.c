#include "args.h"
#include <stdio.h>

int main(int argc, char **argv) {
  args_t args;
  const char *parse_err = NULL;
  if (!parse_args(argc, argv, &args, &parse_err)) {
    free_args(&args);
    printf("Failed to parse args: %s\n", parse_err);
    return -1;
  }

  printf("Hello world (master)\n\n");

  printf("Board size %ux%u\n", args.width, args.height);
  printf("Delay %ums\n", args.delay);
  printf("Timeout %ums\n", args.timeout);
  printf("Seed %d\n", args.seed);

  if (args.view)
    printf("View executable %s", args.view);

  for (int i = 0; args.players[i] != NULL; i++)
    printf("Player %d: %s\n", i + 1, args.players[i]);

  ///// pipe setup mk1 ////

  int pipesw[2][MAX_PLAYERS]; // un array de pipes de read pipes de read
  int pipesr[2][MAX_PLAYERS]; // un array de pipes de write

  for (int i = 0; args.players[i] != NULL; i++) {
    if (pipe(pipesw[i]) == -1 || pipe(pipesr[i]) == -1) {
      return -1; // falla el creado del pipe
    }
  }
  // a cada player va a haber que mandarle sus respectivos pipes de read y write

  int children[MAX_PLAYERS];
  for (int i = 0; args.players[i] != NULL; i++) {
    int pid = fork();
    if (pid == -1) {
      return -1;
    } // falla el fork

    if (pid == 0) {
      /// child proces // deberia correr el codigo del player
      execve("player", args.players,
             enviroment); // todo enviroment variables to pass
    }
    children[i] = pid; // parent guarda el pid de los nenes
  }

  /// en  este punto estan los pipes en pipesr/w y los hijos de players creados
  /// todo: faltan  los pipes de la vista

  free_args(&args);
  return 0;
}
