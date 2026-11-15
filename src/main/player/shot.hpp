#include "pc98.h"

enum shot_flag_t {
	SF_FREE = 0,
	SF_ALIVE = 1,
	SF_REMOVE = 2, // Unblitted on the next frame, then freed
};

struct shot_t {
	uint8_t flag;
	uint8_t age;
	screen_point_t pos[PAGE_COUNT]; // in subpixels, one per VRAM page
	screen_point_t velocity; // in subpixels
	uint8_t patnum;
	bool from_option;
};

static const int SHOT_COUNT = 38;

extern shot_t shots[SHOT_COUNT];

void near shots_update_and_render(void);
