/* ReC98
 * -----
 * Boss explosion animation and boss state reset of TH02's MAIN.EXE, at the
 * beginning of the bullet segment
 */

#pragma option -zCmain_03_TEXT -zPmain_03

#include "platform.h"
#include "x86real.h"
#include "libs/master.lib/master.hpp"
#include "libs/master.lib/pc98_gfx.hpp"
#include "th01/math/polar.hpp"
#include "th02/hardware/pages.hpp"
#include "th02/snd/snd.h"
#include "th02/main/circle.hpp"
#include "th02/main/scroll.hpp"
#include "th02/main/slowdown.hpp"
#include "th02/main/boss/b3.hpp"
#include "th02/main/boss/boss.hpp"
#include "th02/main/boss/bosses.hpp"
#include "th02/main/tile/tile.hpp"
#include "th02/hardware/egc.hpp"

extern "C" void MASTER_RET graph_scroll(
	unsigned line1, unsigned adr1, unsigned adr2
);

unsigned char angle_1E510 = 0x20;
// Uninitialized boss and midboss state, defined in this TU, in the original
// order.
#pragma option -a2
bool midboss_active;
screen_x_t midboss3_left_on_page[STONE_COUNT][PAGE_COUNT];
screen_y_t midboss3_top_on_page[STONE_COUNT][PAGE_COUNT];
int16_t stone_damage[STONE_COUNT];
int patnum_2064E;
int16_t boss_phase_frame;
screen_x_t boss_left_on_page[PAGE_COUNT];
screen_y_t boss_top_on_page[PAGE_COUNT];
int boss_damage;
screen_x_t near* boss_left_on_back_page;
screen_y_t near* boss_top_on_back_page;
stone_flag_t stone_flag[STONE_COUNT];
uint8_t stone_unknown[STONE_COUNT];
uint8_t byte_2066A;
uint8_t byte_2066B;
bool reduce_effects;
uint8_t slowdown_factor;
uint8_t byte_2066E;
uint8_t byte_2066F;
uint8_t group_20670;
uint8_t byte_20671;
uint8_t byte_20672[16]; // Only the first byte is used.
uint32_t sigma_frame;
unsigned int word_20686;
#pragma option -a-

void near playfield_side_columns_clear(void)
{
	grcg_setcolor(GC_RMW, 0);
	graph_accesspage(page_front);
	grcg_byteboxfill_x((16 / 8), 0, (24 / 8), (RES_Y - 1));
	grcg_byteboxfill_x((416 / 8), 0, (424 / 8), (RES_Y - 1));
	graph_accesspage(page_back);
	grcg_byteboxfill_x((16 / 8), 0, (24 / 8), (RES_Y - 1));
	grcg_byteboxfill_x((416 / 8), 0, (424 / 8), (RES_Y - 1));
	grcg_off();
}

void pascal near sub_FFF8(screen_x_t center_x, screen_y_t center_y, int frame)
{
	screen_y_t top;
	int patnum;
	vram_y_t vram_top;
	int i;
	unsigned char angle;
	unsigned char col;
	register pixel_t radius;
	register screen_x_t left;

	if(frame < 0) {
		return;
	}
	if(frame == 0) {
		playfield_side_columns_clear();
	}
	if(frame < 32) {
		if(frame == 1) {
			snd_se_play(18);
		}
		center_x += 7;
		if(frame >= 2) {
			egc_start_copy_noframe();
			frame -= 2;
			radius = (240 - (frame * 8));
			circle_tiles_invalidate(center_x, center_y, radius, 2);
			frame += 2;
			egc_off();
		}
		if(frame < 30) {
			col = (((frame & 1) * 11) + 4);
			grcg_setcolor(GC_RMW, col);
			radius = (240 - (frame * 8));
			circle_dots_put(center_x, center_y, radius, 2);
			grcg_off();
		}
	} else {
		frame -= 32;
		if(frame >= 26) {
			return;
		}
		if(frame == 0) {
			snd_se_play(14);
		}
		if(frame >= 2) {
			frame -= 2;
			radius = (frame * 16);
			egc_start_copy_noframe();
			for(i = 0; i < 0x100; i += 4) {
				angle = (i + frame);
				left = polar_x_fast(center_x, radius, angle);
				top = polar_y(center_y, radius, (angle + angle_1E510));
				if((left >= 16) && (left < 416) && (top > 0) && (top < 384)) {
					tiles_invalidate_rect(left, top, 16, 16);
				}
			}
			frame += 2;
			tiles_egc_render();
			egc_off();
		}
		slowdown_factor = 1;
		if(frame >= 24) {
			return;
		}
		patnum = ((frame / 6) + 88);
		radius = (frame * 16);
		for(i = 0; i < 0x100; i += 4) {
			angle = (i + frame);
			left = polar_x_fast(center_x, radius, angle);
			top = polar_y(center_y, radius, (angle + angle_1E510));
			vram_top = top;
			vram_top += scroll_line;
			if(vram_top >= RES_Y) {
				vram_top -= RES_Y;
			}
			if((left > 16) && (left < 416) && (top > 0) && (top < 384)) {
				super_roll_put_tiny(left, vram_top, patnum);
			}
		}
		slowdown_factor = 2;
		if(frame <= 8) {
			PaletteTone = (((frame & 1) * 70) + 100);
			palette_show();
		}
		if(frame <= 20) {
			i = (frame & 1);
			graph_scroll((RES_Y - scroll_line), ((scroll_line * 40) + i), i);
			slowdown_factor = 3;
		}
	}
}

void sub_1028C(void)
{
	register int i;
	for(i = 0; i < STONE_COUNT; i++) {
		stone_damage[i] = 0;
		stone_flag[i] = SF_DORMANT;
		stone_unknown[i] = 0;
	}
	boss_phase_frame = 0;
	byte_2066A = 0;
	boss_damage = 0;
	byte_2066B = 0;
	sigma_frame = 0;
	word_20686 = 0;
}
