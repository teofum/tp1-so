#ifndef ARGS_H
#define ARGS_H

#include <stdint.h>

#define MAX_PLAYERS 9

/*
 * Command line args for master. Used to initialize game ADT, but we also
 * expose these to the master as it will use the executable names to spawn
 * processes.
 */
typedef struct {
  uint32_t width, height;

  uint32_t delay;
  uint32_t timeout;

  int32_t seed;

  const char *view;
  const char *players[MAX_PLAYERS];
} args_t;

/*
 * Parse CLI args from commandline. Takes an optional error parameter, if not
 * NULL the pointer will be set to an error string if parsing fails.
 */
args_t *parse_args(int argc, char *const *argv, const char **err);

/*
 * Helper function to free heap allocated strings in the args struct and the
 * struct itself.
 */
void free_args(args_t *args);

#endif
