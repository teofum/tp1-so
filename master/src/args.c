#include <args.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>
#include <unistd.h>

extern char *optarg;

int parse_args(int argc, char *const *argv, args_t *args, const char **err) {
  // Initialize default args
  args->width = args->height = 10;
  args->delay = 200;
  args->timeout = 10;
  args->seed = time(NULL);
  args->view = NULL;

  for (int i = 0; i < MAX_PLAYERS; i++)
    args->players[i] = NULL;

  // Parse commandline args
  int ch;
  while ((ch = getopt(argc, argv, "w:h:d:t:s:v:p:")) != -1) {
    printf("Found arg %c with value %s\n", ch, optarg);
    switch (ch) {
    case 'w': {
      int w = atoi(optarg);
      if (w < 10) {
        *err = "Board width must be at least 10";
        return 0;
      }
      args->width = w;
      break;
    }
    case 'h': {
      int h = atoi(optarg);
      if (h < 10) {
        *err = "Board height must be at least 10";
        return 0;
      }
      args->height = h;
      break;
    }
    case 'd': {
      int d = atoi(optarg);
      if (d < 0) {
        *err = "Delay must be a non negative integer";
        return 0;
      }
      args->delay = d;
      break;
    }
    case 't': {
      int t = atoi(optarg);
      if (t <= 0) {
        *err = "Timeout must be a positive integer";
        return 0;
      }
      args->delay = t;
      break;
    }
    case 's': {
      args->seed = atoi(optarg);
      break;
    }
    case 'v': {
      args->view = strdup(optarg);
      break;
    }
    case 'p': {
      int i = 0;
      for (char *word = strtok(optarg, " "); word; word = strtok(NULL, " ")) {
        args->players[i] = strdup(word);

        if (++i > MAX_PLAYERS) {
          *err = "Too many players";
          return 0;
        }
      }
      break;
    }
    }
  }

  if (args->players[0] == NULL) {
    *err = "No players";
    return 0;
  }

  return 1;
}

void free_args(args_t *args) {
  free((void *)args->view);
  for (int i = 0; args->players[i] != NULL; i++)
    free((void *)args->players[i]);
}
