#pragma option -G

/* ReC98
 * -----
 * Code segment #4 of TH02's MAIN.EXE
 */

#include "platform.h"
#include "pc98.h"
#include "th01/math/polar.hpp"
#include "th02/main/playfld.hpp"
#include "th02/main/scroll.hpp"
#include "th02/main/tile/tile.hpp"

#include "th02/main/circle.hpp"

void pascal circle_dots_put(
	screen_x_t center_x, screen_y_t center_y, pixel_t radius, int angle_step
)
{
	unsigned int i;
	unsigned char angle;
	unsigned char size = ((radius >> 6) + 1);
	for(i = 0; i < 0x100; i += angle_step) {
		angle = i;
		circle_dot_left = polar_x_fast(center_x, radius, angle);
		circle_dot_top = polar_y_fast(center_y, radius, angle);
		if(
			(circle_dot_left < 0x1A0) && (circle_dot_left > 0x18) &&
			(circle_dot_top > 8) && (circle_dot_top < 0x180)
		) {
			circle_dot_top += scroll_line;
			if(circle_dot_top >= RES_Y) {
				circle_dot_top -= RES_Y;
			}
			circle_dot_put(size);
		}
	}
}

void pascal circle_tiles_invalidate(
	screen_x_t center_x, screen_y_t center_y, pixel_t radius, int angle_step
)
{
	unsigned int i;
	unsigned char angle;
	unsigned char size = ((radius >> 6) + 1);
	for(i = 0; i < 0x100; i += angle_step) {
		angle = i;
		screen_x_t left = polar_x_fast(center_x, radius, angle);
		screen_y_t top = polar_y_fast(center_y, radius, angle);
		if((left < 0x1A0) && (left > 0x18) && (top > 8) && (top < 0x180)) {
			tiles_invalidate_rect(left, top, size, size);
		}
	}
}
