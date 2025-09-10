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
    usleep(10000); // Wait a bit so the view process has time to actually start
    int ret;
    if (waitpid(view->pid, &ret, WNOHANG)) {
      printf("View exited with code %d\n", ret);
      free(view);
      return NULL;
    }
  } else {
    view->pid = -1;
  }

  view->game = game;
  view->us_delay_after_update = args->delay * 10000;
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
      callback(ret);
  }
}
