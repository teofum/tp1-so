#include "game.h"
#include <spawn.h>
#include <view.h>

#include <stdio.h>
#include <stdlib.h>
#include <sys/wait.h>

struct view_cdt_t {
  int pid;
  useconds_t us_delay_after_update;

  game_t game;
};

view_t view_create(game_t game, args_t *args) {
  view_t view = malloc(sizeof(struct view_cdt_t));
  if (!view)
    return NULL;

  if (args->view) {
    view->pid = spawn_view(args->view, game_state(game));

    // Check that the view process actually started correctly, fail if it didn't
    if (game_wait_for_view(game, 1000000) < 0) {
      perror("Timed out waiting for view");
      int ret;
      waitpid(view->pid, &ret, 0);
      printf("View exited with code %d\n", ret);
      free(view);
      return NULL;
    }
  } else {
    view->pid = -1;
  }

  view->game = game;
  view->us_delay_after_update = args->delay * 1000;
  return view;
}

void view_update(view_t view) {
  // Signal view to update, wait for view to finish and delay
  if (view->pid != -1)
    game_update_view(view->game);

  usleep(view->us_delay_after_update);
}

void view_wait(view_t view, view_wait_callback_t callback) {
  if (view->pid != -1) {
    int ret;
    waitpid(view->pid, &ret, 0);

    if (callback)
      callback(view->pid, ret);
  }

  free(view);
}
