#ifndef ARGS_H
#define ARGS_H

#include <stdint.h>

#define MAX_PLAYERS 9

/*
 * Command line args for all components
 * view/players only need a subset of the master args, so we can
 * reuse this struct and parse function for all of them
 */
typedef struct {
  uint32_t width, height;

  uint32_t delay;
  uint32_t timeout;

  int32_t seed;

  const char *view;
  const char *players[MAX_PLAYERS];
} args_t;

int parse_args(int argc, char *const *argv, args_t *args, const char **err);

void free_args(args_t *args);

#endif
