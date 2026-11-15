/* ReC98
 * -----
 * Pillar objects of TH02's MAIN.EXE, just before the dialog code
 */

#pragma option -zCmain_03_TEXT -zPmain_03 -G

#include "platform.h"
#include "x86real.h"
#include "pc98.h"
#include "libs/master.lib/master.hpp"
#include "libs/master.lib/pc98_gfx.hpp"
#include "th02/snd/snd.h"
#include "th02/main/frames.hpp"
#include "th02/main/scroll.hpp"
#include "th02/main/player/player.hpp"
#include "th02/main/tile/tile.hpp"

enum pillar_flag_t {
	PF_FREE = 0,
	PF_ALIVE = 1,
	PF_REMOVE = 2,
};

struct pillar_t {
	uint8_t flag;
	uint8_t phase;
	screen_x_t left;
	screen_y_t top;
	int frames;
	int duration;
	uint8_t cel;
	uint8_t patnum;
};

static const int PILLAR_COUNT = 12;

// Uninitialized data, in the original order
pillar_t pillars[PILLAR_COUNT];
screen_point_t pillar_cur;
uint8_t pillar_frames_new;
extern void (far *farfp_23A72)(void);
extern void (far *farfp_23A76)(void);

void sub_129DD(void)
{
	register pillar_t near *p = pillars;
	int i;
	for(i = 0; i < PILLAR_COUNT; i++, p++) {
		p->flag = PF_FREE;
	}
	pillar_frames_new = 16;
}

void sub_12A7D(void);
void sub_12B9E(void);

void sub_129FC(void)
{
	farfp_23A72 = sub_12A7D;
	farfp_23A76 = sub_12B9E;
}

void pascal near pillar_add(
	screen_x_t left, screen_y_t top, int duration, int patnum
)
{
	register pillar_t near *p = pillars;
	register int i;

	if((left < 32) || (left >= 416)) {
		return;
	}
	for(i = 0; i < PILLAR_COUNT; i++, p++) {
		if(p->flag == PF_FREE) {
			p->flag = PF_ALIVE;
			p->phase = 1;
			p->left = left;
			p->top = top;
			p->frames = pillar_frames_new;
			p->duration = duration;
			p->cel = 0;
			p->patnum = patnum;
			snd_se_play(6);
			return;
		}
	}
}

void sub_12A7D(void)
{
	register pillar_t near *p = pillars;
	register int i;

	for(i = 0; i < PILLAR_COUNT; i++, p++) {
		if(p->flag == PF_FREE) {
			continue;
		}
		if(p->cel <= 4) {
			tiles_invalidate_rect((p->left - 8), (p->top - 8), 32, 32);
			if((stage_frame & 3) == 0) {
				p->cel++;
			}
		} else {
			tiles_invalidate_rect(p->left, p->top, 16, (RES_Y - p->top));
		}
		if(p->flag == PF_REMOVE) {
			p->flag = PF_FREE;
		}
	}
}

void pascal near pillar_put(pillar_t near *p)
{
	screen_y_t top;
	int patnum;
	register vram_y_t vram_top;

	vram_top = scroll_line;
	top = pillar_cur.y;
	vram_top += pillar_cur.y;
	if(vram_top >= RES_Y) {
		vram_top -= RES_Y;
	}
	if(p->phase < 5) {
		super_roll_put_tiny(pillar_cur.x, vram_top, (p->phase + 91));
	}
	vram_top += 8;
	if(vram_top >= RES_Y) {
		vram_top -= RES_Y;
	}
	top += 8;
	patnum = (p->patnum + p->phase);
	while(top < 384) {
		super_roll_put_tiny(pillar_cur.x, vram_top, patnum);
		vram_top += 16;
		if(vram_top >= RES_Y) {
			vram_top -= RES_Y;
		}
		top += 16;
	}
	if(
		(p->phase == 4) &&
		((pillar_cur.x - 24) < player_topleft.x) &&
		((pillar_cur.x + 8) > player_topleft.x) &&
		(player_topleft.y >= pillar_cur.y) &&
		!player_is_hit
	) {
		player_is_hit = true;
	}
}

void sub_12B9E(void)
{
	screen_x_t left;
	int i;
	int patnum;
	register pillar_t near *p = pillars;
	register vram_y_t vram_top;

	for(i = 0; i < PILLAR_COUNT; i++, p++) {
		if(p->flag != PF_ALIVE) {
			continue;
		}
		pillar_cur.x = p->left;
		pillar_cur.y = p->top;
		if(p->cel < 4) {
			vram_top = (pillar_cur.y - 8);
			vram_top += scroll_line;
			if(vram_top >= RES_Y) {
				vram_top -= RES_Y;
			}
			left = (pillar_cur.x - 8);
			patnum = (p->cel + 30);
			super_roll_put(left, vram_top, patnum);
			p->frames--;
			continue;
		}
		if(p->phase == 1) {
			p->frames--;
			if(p->frames <= 0) {
				p->phase++;
			}
		} else if(p->phase < 4) {
			if((stage_frame & 7) == 0) {
				p->phase++;
			}
			if(p->phase == 4) {
				snd_se_play(7);
			}
		} else if(p->phase == 4) {
			p->duration--;
			if(p->duration == 0) {
				p->phase++;
			}
		} else {
			if((stage_frame & 3) == 0) {
				p->phase++;
			}
			if(p->phase == 9) {
				p->flag = PF_REMOVE;
				continue;
			}
		}
		pillar_put(p);
	}
}
