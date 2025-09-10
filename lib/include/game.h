#ifndef GAME_H
#define GAME_H

#include "args.h"
#include "game_state.h"

/*
 * Game ADT. Allows access to game state and exposes synchronization
 * events.
 */
typedef struct game_cdt_t *game_t;

/*
 * Initialize game. Creates share memory and sync resources and initializes
 * board and player state. Takes an optional error parameter, if not NULL the
 * pointer will be set to an error string if initialization fails.
 */
game_t game_init(args_t *args, const char **err);

/*
 * Connect to an existing game, used by view and players.
 */
game_t game_connect(uint32_t width, uint32_t height);

/*
 * Disconnect a game without destroying any resources.
 */
void game_disconnect(game_t game);

/*
 * Disconnect a game and destroy any resources. Must be called after all
 * view and player processes have exited.
 */
void game_destroy(game_t game);

/*
 * End the game, signaling the view and players to exit.
 */
void game_end(game_t game);

// === State access ===========================================================

/*
 * Get a pointer to the shared game state. Readers should lock the state before
 * accessing it by calling game_will_read_state, and release it afterwards by
 * calling game_did_read_state.
 */
game_state_t *game_state(game_t game);

/*
 * Get a copy of the game state at the time of calling this function. Readers
 * should lock and release the state before and after calling this function,
 * but may access the local copy freely afterwards.
 */
game_state_t game_clone_state(game_t game);

// === Sync functions =========================================================

/*
 * Signals a process will access the game state for reading. Multiple processes
 * may have simultaneous read access as long as a write lock is not in place.
 */
void game_will_read_state(game_t game);

/*
 * Signals a process has finished accessing the game state for reading.
 */
void game_did_read_state(game_t game);

/*
 * Signals a process will access the game state for writing. Writing requires
 * exclusive access to the state.
 */
void game_lock_state_for_writing(game_t game);

/*
 * Signals a process is done accessing the game state for writing.
 */
void game_release_state(game_t game);

/*
 * Blocks until the player's last move has been processed by the master and
 * game state updated accordingly.
 */
void game_wait_move_processed(game_t game, size_t player_idx);

/*
 * Signals a player its last move was processed and the state updated.
 */
void game_post_move_processed(game_t game, size_t player_idx);

/*
 * Signals the master has updated the game state and the view should redraw.
 * Blocks until view has finished drawing.
 */
void game_update_view(game_t game);

/*
 * Blocks until the view should redraw.
 */
void game_wait_view_should_update(game_t game);

/*
 * Signals the view has finished drawing.
 */
void game_post_view_did_update(game_t game);

#endif
