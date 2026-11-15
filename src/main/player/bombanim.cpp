/* ReC98
 * -----
 * Bomb data loading, the bomb circle, and the three bomb animations of TH02's
 * MAIN.EXE
 */

#pragma option -zCmain_01_TEXT -zPmain_01 -G

#include "platform.h"
#include "x86real.h"
#include "pc98.h"
#include "libs/master.lib/master.hpp"
#include "libs/master.lib/pc98_gfx.hpp"
#include "th01/math/polar.hpp"
#include "th02/resident.hpp"
#include "th02/v_colors.hpp"
#include "th02/hardware/pages.hpp"
#include "th02/formats/pi.h"
#include "th02/math/randring.hpp"
#include "th02/snd/snd.h"
#include "th02/main/playfld.hpp"
#include "th02/main/playperf.hpp"
#include "th02/main/scroll.hpp"
#include "th02/main/slowdown.hpp"
#include "th02/main/spark.hpp"
#include "th02/main/bullet/bullet.hpp"
#include "th02/main/hud/hud.hpp"
#include "th02/main/hud/overlay.hpp"
#include "th02/main/player/bomb.hpp"
#include "th02/main/player/player.hpp"
#include "th02/main/tile/tile.hpp"

extern point_t bomb_circle_center;
extern int bomb_circle_frame;
extern bool16 (near pascal *near playchar_bomb_func)(void);


// Uninitialized data, in the original order
// -----------------------------------------

#pragma option -a2
bool bombing;
int bomb_frame;
int bomb_unused; // ZUN bloat
uint8_t far *bomb_b_bft;
uint8_t far *bombs_bft;
uint8_t stage_bombs_used;
point_t bomb_circle_center;
int bomb_circle_frame;
bool16 bomb_circle_done;
vram_y_t bomb_scroll_line_on_page[PAGE_COUNT];

// ZUN bloat: Both arrays have room for 50 elements, but only 48 and 32 are
// used.
screen_point_t bomb_c_columns[50];
uint8_t bomb_particle_age[50];

bool16 (near pascal *near playchar_bomb_func)(void);
screen_point_t bomb_particles[32];
#pragma option -a-
tile_mode_t bomb_a_tile_mode;
RGB8 bomb_a_col0;
RGB8 bomb_a_col3;
tile_mode_t bomb_c_tile_mode;
tile_mode_t bomb_b_tile_mode;
uint8_t bomb_b_row;
// -----------------------------------------

uint8_t total_bombs_used = 0;

void pascal near bomb_circle_point_put(screen_x_t left, screen_y_t top);
void pascal near bomb_particle_put_8(screen_x_t left, screen_y_t top, int cel);
void pascal near bomb_smear_put_8(screen_x_t left, screen_y_t column_bottom);
void pascal near bomb_bft_8tiles_put_8(
	screen_x_t left, screen_y_t top, dots8_t dots
);

extern "C" bool16 pascal near bomb_reimu_a(void);
extern "C" bool16 pascal near bomb_reimu_b(void);
extern "C" bool16 pascal near bomb_reimu_c(void);

void near sub_E178(void)
{
	file_ropen("bombs.bft");
	file_seek(32, 0);
	bombs_bft = reinterpret_cast<uint8_t far *>(hmem_allocbyte(0x438));
	file_read(bombs_bft, 0x438);
	file_close();
	if(resident->shottype == 0) {
		pi_load(1, "bomb1.pi");
		playchar_bomb_func = bomb_reimu_a;
	} else if(resident->shottype == 2) {
		pi_load(1, "bomb3.pi");
		playchar_bomb_func = bomb_reimu_c;
	} else if(resident->shottype == 1) {
		pi_load(1, "bomb2.pi");
		file_ropen("bomb1.bft");
		file_seek(80, 0);
		bomb_b_bft = reinterpret_cast<uint8_t far *>(hmem_allocbyte(0x1440));
		file_read(bomb_b_bft, 0x1440);
		file_close();
		playchar_bomb_func = bomb_reimu_b;
	}
}

void near sub_E24A(void)
{
	pi_free(1);
	if(resident->shottype == 1) {
		HMem<uint8_t>::free(bomb_b_bft);
	}
}

void near sub_E271(void)
{
	stage_bombs_used = 0;
	bombing = false;
}

extern "C" void pascal near player_bomb(void)
{
	if(!bombing && bombs) {
		bombing = true;
		player_invincible_via_bomb = true;
		bombs--;
		hud_bombs_put();
		bomb_frame = 0;
		stage_bombs_used++;
		total_bombs_used++;
		snd_se_play(9);
		bomb_circle_center.x = (PLAYFIELD_LEFT + (PLAYFIELD_W / 2) - 4);
		bomb_circle_center.y = (PLAYFIELD_TOP + (PLAYFIELD_H / 2) - 4);
		bomb_circle_frame = 0;
		bomb_circle_done = false;
		bullets_clear();
	}
}

void near sub_E2D9(void)
{
	pixel_t radius;
	screen_x_t left;
	unsigned char angle;
	register screen_y_t top;
	register int i;

	if(!bombing) {
		return;
	}
	if(bomb_circle_frame > 1) {
		bomb_frame--;
		radius = (256 - (bomb_frame * 8));
		for((i = 0, angle = bomb_frame); i < 64; (i++, angle += 4)) {
			left = polar_x_fast(bomb_circle_center.x, radius, angle);
			top = polar_y_fast(bomb_circle_center.y, radius, angle);
			if((top <= 8) || (top >= PLAYFIELD_BOTTOM)) {
				continue;
			}
			tiles_invalidate_rect(left, top, 8, 8);
		}
		if(bomb_frame >= BOMB_CIRCLE_FRAMES) {
			bomb_circle_frame = 0;
		}
		bomb_frame++;
	}
	if((bomb_circle_done == true) && (bomb_frame <= BOMB_CIRCLE_FRAMES)) {
		tiles_egc_render_all = true;
	}
	if(resident->shottype == 0) {
		if((bomb_frame == 136) || (bomb_frame == 137)) {
			tiles_egc_render_all = true;
		}
	} else if(resident->shottype == 1) {
		if((bomb_frame == 164) || (bomb_frame == 165)) {
			tiles_egc_render_all = true;
		}
	} else {
		if((bomb_frame == 112) || (bomb_frame == 113)) {
			tiles_egc_render_all = true;
		}
	}
}

void near bomb_circle_update_and_render(void)
{
	screen_x_t left;
	screen_y_t top;
	unsigned char angle;
	register int i;
	register pixel_t radius;

	bomb_circle_frame++;
	bomb_scroll_line_on_page[page_back] = scroll_line;
	grcg_setcolor(GC_RMW, V_WHITE);
	radius = (256 - (bomb_frame * 8));
	for((i = 0, angle = bomb_frame); i < 64; (i++, angle += 4)) {
		left = polar_x_fast(bomb_circle_center.x, radius, angle);
		top = polar_y_fast(bomb_circle_center.y, radius, angle);
		bomb_circle_point_put(left, top);
	}
	grcg_off();
}

void pascal near bomb_b_circle_put(int frame)
{
	screen_x_t left;
	screen_y_t top;
	unsigned char angle;
	register int i;
	register pixel_t radius;

	grcg_setcolor(GC_RMW, 14);
	radius = (frame * 32);
	for((i = 0, angle = frame); i < 64; (i++, angle += 4)) {
		left = polar_x_fast(bomb_circle_center.x, radius, angle);
		top = polar_y_fast(bomb_circle_center.y, radius, angle);
		bomb_circle_point_put(left, top);
	}
	grcg_off();
}

void near bomb_particles_update_and_render(void)
{
	register int i;
	register int first = ((bomb_frame & 0xF) * 2);

	for(i = 0; i < 2; i++) {
		bomb_particles[first + i].x = ((randring1_next16() % 384) + 32);
		bomb_particles[first + i].y = ((randring1_next16() % 368) + 16);
		bomb_particle_age[first + i] = 1;
	}
	for(i = 0; i < 32; i++) {
		if(bomb_particle_age[i] == 0) {
			continue;
		}
		if((bomb_frame & 3) == (i & 3)) {
			bomb_particle_age[i]++;
			if(bomb_particle_age[i] >= 5) {
				bomb_particle_age[i] = 0;
				continue;
			}
		}
		bomb_particle_put_8(
			bomb_particles[i].x, bomb_particles[i].y, (bomb_particle_age[i] - 1)
		);
	}
}

void near bombs_bft_put(void)
{
	uint8_t far *p;
	int cel;
	register screen_y_t top;

	cel = ((bomb_frame - 1) >> 1);
	p = (bombs_bft + (cel * 72));
	for(top = PLAYFIELD_TOP; top < (PLAYFIELD_BOTTOM + TILE_H); top += TILE_H) {
		bomb_bft_8tiles_put_8((PLAYFIELD_LEFT + (0 * 8 * TILE_W)), top, p[0]);
		bomb_bft_8tiles_put_8((PLAYFIELD_LEFT + (1 * 8 * TILE_W)), top, p[1]);
		bomb_bft_8tiles_put_8((PLAYFIELD_LEFT + (2 * 8 * TILE_W)), top, p[2]);
		p += 3;
	}
}

#define playfield_fill() \
	grcg_boxfill(PLAYFIELD_LEFT, 0, (PLAYFIELD_RIGHT - 1), (RES_Y - 1))

extern "C" bool16 pascal near bomb_reimu_a(void)
{
	int tmp;
	register screen_y_t top;

	grcg_setcolor(GC_RMW, 0);
	if(bomb_frame == 1) {
		for(tmp = 0; tmp < sizeof(RGB8); tmp++) {
			bomb_a_col0.v[tmp] = Palettes[0].v[tmp];
			bomb_a_col3.v[tmp] = Palettes[3].v[tmp];
		}
		Palettes[3].c.r = 64;
		Palettes[3].c.g = 16;
		Palettes[3].c.b = 64;
		palette_show();
	}
	if(bomb_frame < BOMB_CIRCLE_FRAMES) {
		grcg_setcolor(GC_RMW, 3);
		bombs_bft_put();
	} else if(bomb_frame == BOMB_CIRCLE_FRAMES) {
		Palettes[3].c.r = bomb_a_col3.c.r;
		Palettes[3].c.g = bomb_a_col3.c.g;
		Palettes[3].c.b = bomb_a_col3.c.b;
		Palettes[0].c.r = 128;
		Palettes[0].c.g = 0;
		Palettes[0].c.b = 128;
		palette_show();
		bomb_a_tile_mode = tile_mode;
		tile_mode = TM_NONE;
		playfield_fill();
	} else if(bomb_frame < 136) {
		if(bomb_frame > 64) {
			grcg_setcolor(GC_RMW, 11);
		}
		playfield_fill();
		grcg_off();
		if(bomb_frame <= 64) {
			tmp = (128 - ((bomb_frame * 4) - 128));
			Palettes[0].c.r = tmp;
			Palettes[0].c.g = 0;
			Palettes[0].c.b = tmp;
			if(bomb_frame == 64) {
				PaletteTone = 200;
				palette_show();
				Palettes[0].c.r = 255;
				Palettes[0].c.g = 255;
				Palettes[0].c.b = 255;
			}
			palette_show();
		} else {
			sparks_add(
				(PLAYFIELD_LEFT + 180), (PLAYFIELD_TOP + 174), ((3 << 4) + 2), 1, false
			);
			if(bomb_frame == 70) {
				PaletteTone = 100;
				palette_show();
			} else if(!reduce_effects || (bomb_frame & 1)) {
				top = 90;
				top += scroll_line;
				if(top >= RES_Y) {
					top -= RES_Y;
				}
				pi_put_8(112, top, 1);
			}
			if(bomb_frame >= 86) {
				if((bomb_frame % 3) != 0) {
					tmp = 255;
				} else {
					tmp = 160;
				}
				Palettes[0].c.r = tmp;
				Palettes[0].c.g = tmp;
				Palettes[0].c.b = tmp;
				PaletteTone = ((bomb_frame * 2) - 72);
				palette_show();
				if(bomb_frame > 111) {
					slowdown_factor = 2;
				}
			}
		}
		if(bomb_frame < 86) {
			grcg_setcolor(GC_RMW, V_WHITE);
			if((bomb_frame & 3) == 0) {
				snd_se_play(15);
			}
			bomb_particles_update_and_render();
		}
		if(bomb_frame == 86) {
			snd_se_play(17);
		}
	} else if(bomb_frame == 136) {
		Palettes[0].c.r = bomb_a_col0.c.r;
		Palettes[0].c.g = bomb_a_col0.c.g;
		Palettes[0].c.b = bomb_a_col0.c.b;
		palette_show();
		snd_se_play(16);
		tile_mode = bomb_a_tile_mode;
		slowdown_factor = 1;
		PaletteTone = 200;
		palette_show();
		bullets_clear();
	} else if(bomb_frame < 156) {
		PaletteTone = (200 - ((bomb_frame * 5) - 680));
		palette_show();
	} else {
		grcg_off();
		return true;
	}
	grcg_off();
	return false;
}

#pragma option -a2

struct bomb_c_speeds_t {
	pixel_t v[48];
};


extern "C" bool16 pascal near bomb_reimu_c(void)
{
	screen_y_t top;
	int i;
	bomb_c_speeds_t speeds = {
		4, 3, 3, 2, 2, 1, 1, 0, 0, 2, 3, 2, 1, 0, 1, 2, 1, 0, 0, 0, 2, 3, 3, 2,
		3, 1, 2, 0, 0, 1, 1, 2, 2, 3, 1, 2, 3, 2, 1, 0, 1, 1, 2, 2, 3, 3, 4, 4,
	};

	grcg_setcolor(GC_RMW, 2);
	if(bomb_frame < BOMB_CIRCLE_FRAMES) {
		bombs_bft_put();
	} else if(bomb_frame == BOMB_CIRCLE_FRAMES) {
		bomb_c_tile_mode = tile_mode;
		tile_mode = TM_NONE;
		playfield_fill();
		for(i = 0; i < 48; i++) {
			bomb_c_columns[i].y = 20;
		}
		snd_se_play(17);
	} else if(bomb_frame < 100) {
		playfield_fill();
		if(bomb_frame >= 80) {
			PaletteTone = ((bomb_frame * 5) - 300);
			palette_show();
			slowdown_factor = 2;
		}
		grcg_setcolor(GC_RMW, 14);
		bomb_particles_update_and_render();
		grcg_setcolor(GC_RMW, 1);
		for(i = 0; i < 48; i++) {
			bomb_c_columns[i].y += speeds.v[i];
			bomb_smear_put_8(((i * 8) + 32), bomb_c_columns[i].y);
		}
	} else if(bomb_frame < 112) {
		PaletteTone = 100;
		palette_show();
		if(bomb_frame & 1) {
			grcg_off();
			top = 16;
			top += scroll_line;
			if(top >= RES_Y) {
				top -= RES_Y;
			}
			pi_put_8(32, top, 1);
			snd_se_play(16);
		} else {
			grcg_setcolor(GC_RMW, V_WHITE);
			playfield_fill();
		}
	} else if(bomb_frame == 112) {
		tile_mode = bomb_c_tile_mode;
		slowdown_factor = 1;
		PaletteTone = 200;
		palette_show();
		bullets_clear();
	} else if(bomb_frame < 128) {
		PaletteTone = (200 - ((bomb_frame * 5) - 540));
		palette_show();
	} else {
		grcg_off();
		return true;
	}
	grcg_off();
	return false;
}

void pascal near bomb_b_row_put(int row)
{
	int y;
	register tram_x_t x;
	register unsigned int atrb;

	row *= 288;
	for(y = 1; y <= 23; y++) {
		for(x = 4; x < 52; row++) {
			if(bomb_b_bft[row] & 0xF0) {
				atrb = (TX_RED | TX_REVERSE);
			} else {
				atrb = TX_WHITE;
			}
			text_putsa(x, y, "  ", atrb);
			x += 2;
			if(bomb_b_bft[row] & 0x0F) {
				atrb = (TX_RED | TX_REVERSE);
			} else {
				atrb = TX_WHITE;
			}
			text_putsa(x, y, "  ", atrb);
			x += 2;
		}
	}
}

extern "C" bool16 pascal near bomb_reimu_b(void)
{
	register screen_y_t top;

	grcg_setcolor(GC_RMW, V_WHITE);
	if(bomb_frame < BOMB_CIRCLE_FRAMES) {
		bombs_bft_put();
	} else if(bomb_frame == BOMB_CIRCLE_FRAMES) {
		bomb_b_row = 0;
		bomb_b_tile_mode = tile_mode;
		tile_mode = TM_NONE;
		playfield_fill();
	} else if(bomb_frame < 132) {
		if((bomb_frame > BOMB_CIRCLE_FRAMES) && ((bomb_frame & 3) == 0)) {
			if((bomb_frame & 7) == 4) {
				snd_se_play(16);
			}
			bomb_b_row_put(bomb_b_row);
			bomb_b_row++;
			if(bomb_b_row >= 18) {
				overlay_wipe();
				bomb_frame = 132;
			}
		}
		playfield_fill();
		grcg_setcolor(GC_RMW, 4);
		bomb_particles_update_and_render();
	} else if(bomb_frame < 164) {
		playfield_fill();
		slowdown_factor = 2;
		if(!reduce_effects || (bomb_frame & 1)) {
			grcg_off();
			top = 84;
			top += scroll_line;
			if(top >= RES_Y) {
				top -= RES_Y;
			}
			pi_put_8(32, top, 1);
		}
		if((bomb_frame & 3) == 0) {
			snd_se_play(15);
		}
		if(bomb_frame <= 140) {
			bomb_b_circle_put(bomb_frame - 132);
		} else if(bomb_frame <= 148) {
			bomb_b_circle_put(bomb_frame - 140);
		} else if(bomb_frame <= 156) {
			bomb_b_circle_put(bomb_frame - 148);
		} else {
			bomb_b_circle_put(bomb_frame - 156);
		}
		PaletteTone = (200 - ((bomb_frame * 3) - 396));
		palette_show();
	} else if(bomb_frame == 164) {
		snd_se_play(16);
		tile_mode = bomb_b_tile_mode;
		slowdown_factor = 1;
		PaletteTone = 200;
		palette_show();
		bullets_clear();
	} else if(bomb_frame < 184) {
		PaletteTone = (200 - ((bomb_frame * 5) - 820));
		palette_show();
	} else {
		grcg_off();
		return true;
	}
	grcg_off();
	return false;
}

// The bomb renderers follow in the same TU (one LEDATA chunk spans the
// boundary), compiled without -G.
#pragma option -G-

#include "th02/main/player/bomb.cpp"
