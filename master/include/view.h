#ifndef VIEW_H
#define VIEW_H

#include <args.h>
#include <game.h>

/*
 * View ADT
 */
typedef struct view_cdt_t *view_t;

/*
 * Callback function to run some code when the view exits
 */
typedef void (*view_wait_callback_t)(int ret);

view_t view_create(game_t game, args_t *args);

void view_update(view_t view);

void view_wait(view_t view, view_wait_callback_t callback);

#endif
