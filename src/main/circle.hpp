// Dotted circles drawn with the 16-pixel dot sprites from SPARKS.
// ---------------------------------------------------------------

#include "pc98.h"

// Position of the next dot drawn by circle_dot_put().
extern screen_x_t circle_dot_left;
extern vram_y_t circle_dot_top;

// Draws a dot of the given [size] at ([circle_dot_left], [circle_dot_top]).
void pascal circle_dot_put(int size);

void pascal circle_dots_put(
	screen_x_t center_x, screen_y_t center_y, pixel_t radius, int angle_step
);
void pascal circle_tiles_invalidate(
	screen_x_t center_x, screen_y_t center_y, pixel_t radius, int angle_step
);
