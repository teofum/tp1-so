#include <graphics.h>

#include <stdint.h>
#include <stdlib.h>

#include <curses.h>

void gfx_init() {
  // TERM env var is lost when process is spawned from the provided master,
  // causing ncurses init to fail. We set it manually to work around this.
  setenv("TERM", "xterm-256color", 0);

  (void)initscr();
  (void)nonl();

  if (has_colors()) {
    start_color();

    for (int i = 0; i < 9; i++) {
      int16_t v = 300 + i * 50;
      init_color(C_GRAY + i, v, v, v);
      init_pair(CP_GRAYSCALE + i, C_GRAY + i, COLOR_BLACK);
    }

    init_pair(CP_PLAYER + 0, COLOR_RED, COLOR_BLACK);
    init_pair(CP_PLAYER + 1, COLOR_GREEN, COLOR_BLACK);
    init_pair(CP_PLAYER + 2, COLOR_YELLOW, COLOR_BLACK);
    init_pair(CP_PLAYER + 3, COLOR_BLUE, COLOR_BLACK);
    init_pair(CP_PLAYER + 4, COLOR_MAGENTA, COLOR_BLACK);
    // TODO
  }
}
