/* ReC98
 * -----
 * Extra Stage: midboss and the boss, Evil Eye Σ
 */

#pragma option -zCmain_03_TEXT -zPmain_03 -G

#include "platform.h"
#include "x86real.h"
#include "pc98.h"
#include "libs/master.lib/master.hpp"
#include "libs/master.lib/pc98_gfx.hpp"
#include "th01/math/polar.hpp"
#include "th02/v_colors.hpp"
#include "th02/resident.hpp"
#include "th02/core/globals.hpp"
#include "th02/hardware/input.hpp"
#include "th02/hardware/pages.hpp"
#include "th02/math/randring.hpp"
#include "th02/math/vector.hpp"
#include "th02/snd/snd.h"
#include "th02/main/circle.hpp"
#include "th02/main/frames.hpp"
#include "th02/main/playfld.hpp"
#include "th02/main/score.hpp"
#include "th02/main/scroll.hpp"
#include "th02/main/slowdown.hpp"
#include "th02/main/boss/boss.hpp"
#include "th02/main/boss/bosses.hpp"
#include "th02/main/bullet/bullet.hpp"
#include "th02/main/dialog/dialog.hpp"
#include "th02/main/player/player.hpp"
#include "th02/main/stage/bonus.hpp"
#include "th02/main/tile/tile.hpp"
#include "th02/sprites/bullet16.h"

extern "C" void MASTER_RET graph_scroll(
	unsigned line1, unsigned adr1, unsigned adr2
);

extern screen_x_t shot_target_left;
extern screen_y_t shot_target_top;
extern uint8_t pillar_frames_new;
extern int patnum_2064E;
extern uint8_t byte_2066A;

int pascal near shots_hittest(
	screen_x_t left, screen_y_t top, pixel_t w, pixel_t h
);
void pascal near pillar_add(
	screen_x_t left, screen_y_t top, int duration, int patnum
);
void sub_13ABB(char far *fn);
void near sub_1A6C5(void);
void near dialog_pre(void);
void near dialog_post(void);
void near dialog_script_extra_pre_intro_animate(void);
void scoredat_extra_clear_save(void);
int GameExecl(char *binary_fn);
void shots_clear(void);

// State
// -----

struct sigma_circle_t {
	uint8_t flag; // 1 = growing sprite, 2..3 = expanding, 4 = vanishing
	uint8_t frame;
	screen_x_t center_x;
	screen_y_t center_y;
	pixel_t radius_max;
	uint8_t cel;
	uint8_t radius;
};

static const int SIGMA_CIRCLE_COUNT = 16;

struct sigma_pillar_offsets_t {
	pixel_t v[5];
};

// -----

// Midboss
// -------

struct midbossx_tile_t {
	uint8_t flag; // Nonzero: current tile image index
	uint8_t frame;
	screen_x_t left;
	vram_y_t top;
	uint8_t group;
	unsigned char angle;
	uint8_t speed;
	int8_t padding;
};

static const int MIDBOSSX_TILE_COUNT = 30;

struct midbossx_tile_images_t {
	uint8_t v[7];
};

// Uninitialized data, in the original order
#pragma option -a2
bool midbossx_active;
screen_x_t sigma_bullet_left;
screen_y_t sigma_bullet_top;
midbossx_tile_t midbossx_tiles[MIDBOSSX_TILE_COUNT];
screen_point_t sigma_topleft;
uint8_t midbossx_spread;
sigma_circle_t sigma_circles[SIGMA_CIRCLE_COUNT];
uint8_t sigma_circle_frame_mask;
int8_t sigma_circle_hitbox_shrink;
uint8_t sigma_beam_w;
uint8_t sigma_beam_vram_left;
uint8_t sigma_beam_vram_right;
screen_x_t sigma_beam_left;
screen_x_t sigma_beam_right;
uint8_t sigma_15D56_frames;
int8_t sigma_15D56_velocity;
int8_t sigma_15EF7_velocity;
screen_x_t sigma_15F95_center_x;
screen_y_t sigma_15F95_center_y;
unsigned char sigma_15F95_angle;
screen_x_t sigma_1619C_left;
uint8_t sigma_1619C_round;
int8_t sigma_pillar_i;
screen_point_t sigma_16421_velocity;
screen_x_t sigma_16421_left_1;
screen_y_t sigma_16421_top;
pixel_t sigma_16421_velocity_x_2;
screen_x_t sigma_16421_left_2;
int8_t sigma_165A5_velocity;
unsigned char sigma_16650_angle;
uint8_t sigma_phase;
uint8_t sigma_patterns_done;
uint8_t sigma_pattern;
void (near *sigma_pattern_funcs[3])(void);
int sigma_phase_damage;
unsigned char sigma_ring_angle;
#pragma option -a-

void pascal near midbossx_tile_add(
	screen_x_t left,
	vram_y_t top,
	uint8_t group,
	unsigned char angle,
	uint8_t speed
)
{
	register midbossx_tile_t near *p;
	int i;

	if(top < 0) {
		top += RES_Y;
	} else if(top >= RES_Y) {
		top -= RES_Y;
	}
	p = midbossx_tiles;
	for(i = 0; i < MIDBOSSX_TILE_COUNT; (i++, p++)) {
		if(p->flag == 0) {
			p->flag = 1;
			p->frame = 0;
			p->left = left;
			p->top = top;
			p->group = group;
			p->angle = angle;
			p->speed = speed;
			return;
		}
	}
}

void near midbossx_tiles_update(void)
{
	midbossx_tile_images_t images = { 0, 1, 2, 0x10, 0x11, 0x12, 9 };
	register midbossx_tile_t near *p = midbossx_tiles;
	register int i;

	for(i = 0; i < MIDBOSSX_TILE_COUNT; (i++, p++)) {
		if(p->flag == 0) {
			continue;
		}
		p->frame++;
		if(p->frame & 7) {
			continue;
		}
		tile_ring_set_and_put_both_8(p->left, p->top, images.v[p->flag]);
		p->flag++;
		if(p->flag >= 7) {
			p->top -= scroll_line;
			if(p->top < 0) {
				p->top += RES_Y;
			}
			bullets_add_pellet(
				(p->left + 4), (p->top + 4), p->angle, p->group, p->speed
			);
			p->flag = 0;
		}
	}
}

int midbossx_invalidate(void)
{
	return midbossx_active;
}

#define midbossx_cross_add(center_x, center_y, group) { \
	if(midbossx_spread == 0) { \
		midbossx_tile_add(center_x, center_y, group, 0x00, 100); \
	} else { \
		midbossx_tile_add( \
			(center_x - midbossx_spread), center_y, group, 0x00, 100 \
		); \
		midbossx_tile_add( \
			center_x, (center_y - midbossx_spread), group, 0x00, 100 \
		); \
		midbossx_tile_add( \
			(midbossx_spread + center_x), center_y, group, 0x00, 100 \
		); \
		midbossx_tile_add( \
			center_x, (midbossx_spread + center_y), group, 0x00, 100 \
		); \
	} \
	midbossx_spread += 16; \
}

void midbossx_update_and_render(void)
{
	boss_phase_frame++;
	if(boss_phase_frame == 1) {
		midbossx_spread = 0;
		midbossx_active = true;
	} else if(boss_phase_frame < 96) {
		if((boss_phase_frame % 16) == 0) {
			midbossx_cross_add(128, 128, 0x21);
		}
	} else if(boss_phase_frame == 96) {
		midbossx_spread = 0;
	} else if(boss_phase_frame < 192) {
		if((boss_phase_frame % 16) == 0) {
			if(midbossx_spread == 0) {
				midbossx_tile_add(304, 0, 0x24, 0x00, 100);
			} else {
				midbossx_tile_add((304 - midbossx_spread), 0, 0x24, 0x00, 100);
				midbossx_tile_add(304, -midbossx_spread, 0x24, 0x00, 100);
				midbossx_tile_add((midbossx_spread + 304), 0, 0x24, 0x00, 100);
				midbossx_tile_add(304, midbossx_spread, 0x24, 0x00, 100);
			}
			midbossx_spread += 16;
		}
	} else if(boss_phase_frame == 192) {
		midbossx_spread = 0;
	} else if(boss_phase_frame < 288) {
		if((boss_phase_frame % 16) == 0) {
			midbossx_cross_add(192, 336, 0x25);
		}
	}
	if(boss_phase_frame > 420) {
		boss_phase_frame = 0;
		midbossx_active = false;
	}
	midbossx_tiles_update();
}

// Boss
// ----

int pascal near sigma_circle_add(
	screen_x_t center_x, screen_y_t center_y, pixel_t radius_max
)
{
	sigma_circle_t *p;
	register int i;

	p = sigma_circles;
	if(
		(center_x > 0) && (center_x < 444) &&
		(center_y > 0) && (center_y < 400)
	) {
		for(i = 0; i < SIGMA_CIRCLE_COUNT; (i++, p++)) {
			if(p->flag != 0) {
				continue;
			}
			p->flag = 1;
			p->frame = 0;
			p->center_x = center_x;
			p->center_y = center_y;
			p->radius_max = radius_max;
			p->cel = 26;
			p->radius = 64;
			snd_se_play(9);
			return 0;
		}
	}
	return 1;
}

void near sigma_circles_advance(void)
{
	sigma_circle_t *p;
	int i;

	p = sigma_circles;
	for(i = 0; i < SIGMA_CIRCLE_COUNT; (i++, p++)) {
		if(p->flag == 2) {
			p->flag++;
		}
	}
}

void near sigma_circles_update_and_render(void)
{
	sigma_circle_t *p;
	register int i;

	p = sigma_circles;
	for(i = 0; i < SIGMA_CIRCLE_COUNT; (i++, p++)) {
		if(p->flag == 0) {
			continue;
		}
		p->frame++;
		if(p->flag == 1) {
			if((p->frame & sigma_circle_frame_mask) == 0) {
				p->cel++;
				if(p->cel >= 34) {
					p->flag++;
					snd_se_play(3);
				}
				p->frame = 0;
				p->radius -= 8;
				grcg_setcolor(GC_RMW, V_WHITE);
				grcg_circle(p->center_x, p->center_y, p->radius);
				grcg_off();
			}
			if(p->flag == 1) {
				super_put_rect((p->center_x - 16), (p->center_y - 16), p->cel);
			}
		} else if(p->flag == 3) {
			if(page_back == (i & 1)) {
				grcg_setcolor(GC_RMW, V_WHITE);
				grcg_circlefill(p->center_x, p->center_y, p->radius);
				grcg_off();
				if(p->radius < p->radius_max) {
					p->radius += 16;
				}
			}
			if(p->frame > 32) {
				p->flag++;
				p->frame = 0;
				p->radius_max = p->radius;
				p->radius = 8;
			}
			if(
				(
					((p->center_x - p->radius) + sigma_circle_hitbox_shrink) <
					player_left_on_page[page_front]
				) && (
					(((p->center_x + p->radius) - sigma_circle_hitbox_shrink) - 32) >
					player_left_on_page[page_front]
				) && (
					((p->center_y - p->radius) + sigma_circle_hitbox_shrink) <
					player_top_on_page[page_front]
				) && (
					(((p->center_y + p->radius) - sigma_circle_hitbox_shrink) - 32) >
					player_top_on_page[page_front]
				)
			) {
				player_is_hit = true;
			}
		} else if(p->flag == 4) {
			if(page_back == (i & 1)) {
				grcg_setcolor(GC_RMW, V_WHITE);
				grcg_circlefill(p->center_x, p->center_y, p->radius_max);
				grcg_off();
				grcg_setcolor(GC_RMW, 0);
				grcg_circlefill(p->center_x, p->center_y, p->radius);
				grcg_off();
				if(p->radius >= p->radius_max) {
					p->flag = 0;
				} else {
					p->radius += 16;
				}
			}
		}
	}
}

void near sigma_put(void)
{
	super_put_rect(sigma_topleft.x, sigma_topleft.y, patnum_2064E);
	super_put_rect((sigma_topleft.x + 64), sigma_topleft.y, (patnum_2064E + 1));
}

void near sigma_hittest_and_put(void)
{
	int damage;
	int patnum;

	patnum = patnum_2064E;
	if(!byte_2066A) {
		if((damage = shots_hittest(
			(*boss_left_on_back_page + 36), (*boss_top_on_back_page + 32), 56, 32
		)) != 0) {
			patnum_2064E = 130;
			boss_damage += damage;
		}
		if(
			(player_left_on_page[page_front] > (sigma_topleft.x + 8)) &&
			(player_left_on_page[page_front] < (sigma_topleft.x + 88)) &&
			(player_top_on_page[page_front] > sigma_topleft.y) &&
			(player_top_on_page[page_front] < (sigma_topleft.y + 64))
		) {
			player_is_hit = true;
		}
	}
	sigma_put();
	patnum_2064E = patnum;
}

void sigma_bg_render(void)
{
	int unused = 8;

	boss_left_on_back_page = &boss_left_on_page[page_back];
	boss_top_on_back_page = &boss_top_on_page[page_back];
	*boss_left_on_back_page = boss_left_on_page[page_front];
	*boss_top_on_back_page = boss_top_on_page[page_front];
	egc_off();
	grcg_setcolor(GC_RMW, 0);
	grcg_byteboxfill_x(
		PLAYFIELD_VRAM_LEFT,
		PLAYFIELD_TOP,
		(PLAYFIELD_VRAM_RIGHT - 1),
		(PLAYFIELD_BOTTOM - 1)
	);
	grcg_off();
	sigma_circles_advance();
}

#define sigma_beam_hittest() { \
	if( \
		(player_left_on_page[page_front] > sigma_beam_left) && \
		(player_left_on_page[page_front] < sigma_beam_right) \
	) { \
		player_is_hit = true; \
	} \
}

#define sigma_beam_put(col) { \
	grcg_setcolor(GC_RMW, col); \
	grcg_byteboxfill_x( \
		sigma_beam_vram_left, 16, sigma_beam_vram_right, (PLAYFIELD_BOTTOM - 1) \
	); \
	grcg_off(); \
}

int near sigma_defeat_animate(void)
{
	register int i;

	if(boss_phase_frame < 10) {
		sigma_put();
		return 0;
	}
	if(boss_phase_frame == 10) {
		sigma_beam_w = 8;
		sigma_put();
		snd_se_play(9);
	} else if(boss_phase_frame < 90) {
		sigma_put();
		if(page_back == 0) {
			grcg_setcolor(GC_RMW, V_WHITE);
			grcg_circlefill((sigma_topleft.x + 64), (sigma_topleft.y + 64), 24);
			grcg_off();
		}
		if((boss_phase_frame >= 50) && (page_back != 0)) {
			grcg_setcolor(GC_RMW, V_WHITE);
			grcg_vline(
				(sigma_topleft.x + 64), PLAYFIELD_TOP, (PLAYFIELD_BOTTOM - 1)
			);
			grcg_off();
		}
		if(boss_phase_frame == 89) {
			snd_se_play(3);
		}
	} else if(boss_phase_frame < 100) {
		grcg_setcolor(GC_RMW, V_WHITE);
		grcg_byteboxfill_x(
			(((sigma_topleft.x + 64) - sigma_beam_w) / 8),
			16,
			((sigma_beam_w + sigma_topleft.x + 64) / 8),
			(PLAYFIELD_BOTTOM - 1)
		);
		grcg_off();
		if(
			(((sigma_topleft.x + 56) - sigma_beam_w) <
			player_left_on_page[page_front]) &&
			(player_left_on_page[page_front] <
			(sigma_beam_w + sigma_topleft.x + 40))
		) {
			player_is_hit = true;
		}
		sigma_beam_w += 8;
		sigma_put();
	} else if(boss_phase_frame == 100) {
		sigma_beam_left = ((sigma_topleft.x + 56) - sigma_beam_w);
		sigma_beam_right = (sigma_beam_w + sigma_topleft.x + 40);
		sigma_beam_vram_left = (((sigma_topleft.x + 64) - sigma_beam_w) / 8);
		sigma_beam_vram_right = (((sigma_beam_w + sigma_topleft.x + 64) / 8) - 1);
		patnum_2064E = 134;
	} else if(boss_phase_frame <= 140) {
		sigma_beam_put(V_WHITE);
		sigma_put();
		sigma_beam_hittest();
	} else if(boss_phase_frame < 180) {
		sigma_beam_put(V_WHITE);
		if((boss_phase_frame % 10) == 0) {
			patnum_2064E += 2;
		}
		sigma_put();
		sigma_beam_hittest();
		i = (boss_phase_frame & 1);
		graph_scroll((RES_Y - scroll_line), ((scroll_line * 40) + i), i);
		slowdown_factor = 2;
	} else if(boss_phase_frame <= 220) {
		slowdown_factor = 1;
		sigma_beam_put(V_WHITE);
		sigma_beam_hittest();
	} else if(boss_phase_frame < 224) {
		sigma_beam_put(0);
	} else {
		return 1;
	}
	return 0;
}

#define player_left_of_center() \
	(player_topleft.x < (PLAYFIELD_LEFT + (PLAYFIELD_W / 2) - (PLAYER_W / 2)))

#define sigma_15D56_pellet_add(angle) { \
	bullets_add_pellet( \
		(randring2_next8_and(0x7F) + sigma_topleft.x), \
		(sigma_topleft.y + 64), \
		angle, \
		BG_1, \
		(randring2_next8_and(0x1F) + ((1 << 4) + 14)) \
	); \
}

void near sigma_15D56(void)
{
	unsigned char angle;
	register int i;

	if(boss_phase_frame < 20) {
		return;
	}
	if(boss_phase_frame == 20) {
		sigma_15D56_frames = 0;
		sigma_15D56_velocity = (player_left_of_center() ? 1 : -1);
	}
	if((boss_phase_frame % 4) == 0) {
		snd_se_play(3);
	}
	if(boss_phase_frame < 148) {
		*boss_left_on_back_page += sigma_15D56_velocity;
		if(boss_phase_frame & 1) {
			sigma_15D56_pellet_add(0x40);
		}
	} else if(boss_phase_frame < 212) {
		*boss_left_on_back_page -= (sigma_15D56_velocity * 4);
		if(boss_phase_frame & 1) {
			sigma_15D56_pellet_add(0x40);
		}
	} else if(boss_phase_frame < 340) {
		*boss_left_on_back_page += sigma_15D56_velocity;
		if(boss_phase_frame & 1) {
			sigma_15D56_pellet_add(0x40);
		}
	} else if(boss_phase_frame < 440) {
		sigma_15D56_frames++;
		angle = (
			(randring2_next8() % (sigma_15D56_frames * 2)) +
			(0x40 - sigma_15D56_frames)
		);
		if((boss_phase_frame & 3) == 0) {
			for(i = 0; i < 3; i++) {
				sigma_15D56_pellet_add(angle);
			}
		}
	} else {
		boss_phase_frame = 0;
	}
}

void near sigma_15E84(void)
{
	if(boss_phase_frame < 100) {
		return;
	}
	if(boss_phase_frame == 100) {
		sigma_circle_frame_mask = 7;
		sigma_circle_hitbox_shrink = 4;
		sigma_circle_add(128, 320, 64);
	} else if(boss_phase_frame == 120) {
		sigma_circle_add(320, 320, 64);
	} else if(
		(boss_phase_frame > 130) &&
		(boss_phase_frame < 150) &&
		((boss_phase_frame % 4) == 0)
	) {
		bullets_add_pellet(
			sigma_bullet_left,
			sigma_bullet_top,
			0x00,
			BG_5_SPREAD_MEDIUM_AIMED,
			((7 << 4) + 8)
		);
	}
	if(boss_phase_frame > 200) {
		boss_phase_frame = 0;
	}
}

int near sigma_15EF7(void)
{
	if(boss_phase_frame < 50) {
		return 1;
	}
	if(boss_phase_frame == 50) {
		sigma_15EF7_velocity = (player_left_of_center() ? 1 : -1);
	}
	if(boss_phase_frame < 114) {
		*boss_left_on_back_page += sigma_15EF7_velocity;
		(*boss_top_on_back_page)++;
	} else if(boss_phase_frame < 242) {
		*boss_left_on_back_page -= sigma_15EF7_velocity;
	} else if(boss_phase_frame < 306) {
		*boss_left_on_back_page += sigma_15EF7_velocity;
		(*boss_top_on_back_page)--;
	} else {
		boss_phase_frame = 0;
	}
	return 0;
}

void near sigma_15F6F(void)
{
	if(sigma_15EF7()) {
		return;
	}
	if((boss_phase_frame & 0x1F) == 0) {
		bullets_add_pellet(
			sigma_bullet_left,
			sigma_bullet_top,
			randring2_next8(),
			BG_32_RING,
			((5 << 4) + 5)
		);
	}
}

#define sigma_15F95_circle_put(col) { \
	grcg_setcolor(GC_RMW, col); \
	grcg_circle(sigma_15F95_center_x, sigma_15F95_center_y, 112); \
	grcg_off(); \
}

void near sigma_15F95(void)
{
	unsigned char angle;
	register screen_x_t left;
	register screen_y_t top;

	if(boss_phase_frame < 50) {
		return;
	}
	if(boss_phase_frame == 50) {
		snd_se_play(9);
	}
	if(boss_phase_frame < 100) {
		patnum_2064E = ((page_back * 4) + 128);
		return;
	}
	if(boss_phase_frame == 100) {
		sigma_15F95_center_x = (player_topleft.x + 16);
		sigma_15F95_center_y = (player_topleft.y + 16);
		sigma_circle_frame_mask = 3;
		sigma_circle_hitbox_shrink = -4;
		sigma_15F95_angle = 0x00;
		patnum_2064E = 128;
	}
	if(boss_phase_frame < 130) {
		if((boss_phase_frame % 8) == 0) {
			grcg_setcolor(GC_RMW, V_WHITE);
			grcg_circle(sigma_15F95_center_x, sigma_15F95_center_y, 112);
		} else if((boss_phase_frame % 8) == 2) {
			grcg_setcolor(GC_RMW, 0);
			grcg_circle(sigma_15F95_center_x, sigma_15F95_center_y, 112);
		}
		grcg_off();
	} else if(boss_phase_frame < 450) {
		if(page_back == 0) {
			sigma_15F95_circle_put(V_WHITE);
		}
		if(boss_phase_frame >= 150) {
			if((boss_phase_frame % 12) == 0) {
				left = polar_x_fast(sigma_15F95_center_x, 112, sigma_15F95_angle);
				top = polar_y_fast(sigma_15F95_center_y, 112, sigma_15F95_angle);
				sigma_circle_add(left, top, 24);
				angle = (sigma_15F95_angle + 0x80);
				left = polar_x_fast(sigma_15F95_center_x, 112, angle);
				top = polar_y_fast(sigma_15F95_center_y, 112, angle);
				sigma_circle_add(left, top, 24);
				sigma_15F95_angle -= 0x10;
			}
			if((boss_phase_frame >= 200) && ((boss_phase_frame & 0xF) == 0)) {
				bullets_add_pellet(
					sigma_bullet_left,
					sigma_bullet_top,
					randring2_next8(),
					BG_32_RING,
					(5 << 4)
				);
			}
		}
	} else if(page_back == 0) {
		sigma_15F95_circle_put(0);
		boss_phase_frame = 0;
	}
}

void near sigma_16176(void)
{
	if(sigma_15EF7()) {
		return;
	}
	if((boss_phase_frame & 7) == 0) {
		bullets_add_pellet(
			sigma_bullet_left,
			sigma_bullet_top,
			randring2_next8(),
			BG_16_RING,
			((3 << 4) + 2)
		);
	}
}

void near sigma_1619C(void)
{
	if(boss_phase_frame < 50) {
		return;
	}
	if(boss_phase_frame == 50) {
		snd_se_play(9);
	}
	if(boss_phase_frame < 100) {
		patnum_2064E = ((page_back * 4) + 128);
		return;
	}
	if(boss_phase_frame == 100) {
		patnum_2064E = 128;
		sigma_1619C_left = PLAYFIELD_LEFT;
		sigma_1619C_round = 0;
		bullet_special.u3.turns_max = 1;
	}
	if((boss_phase_frame & 0xF) == 0) {
		bullets_add_16x16(
			sigma_1619C_left,
			(PLAYFIELD_TOP + 8),
			0x40,
			BSM_BOUNCE_LEFT_RIGHT_TOP_BOTTOM,
			(sigma_1619C_round + PAT_BULLET16_BILLIARD_BALL_RED),
			(4 << 4)
		);
		bullets_add_16x16(
			((PLAYFIELD_RIGHT + BULLET16_W) - sigma_1619C_left),
			(PLAYFIELD_TOP + 8),
			0x40,
			BSM_BOUNCE_LEFT_RIGHT_TOP_BOTTOM,
			(sigma_1619C_round + PAT_BULLET16_BILLIARD_BALL_RED),
			(4 << 4)
		);
		sigma_1619C_left += BULLET16_W;
		if(sigma_1619C_left == (PLAYFIELD_LEFT + (PLAYFIELD_W / 2))) {
			pillar_frames_new = 32;
			pillar_add((sigma_topleft.x + 60), sigma_bullet_top, 1, 0x6F);
			pillar_frames_new = 48;
			pillar_add((sigma_topleft.x + 44), sigma_bullet_top, 1, 0x6F);
			pillar_add((sigma_topleft.x + 76), sigma_bullet_top, 1, 0x6F);
			pillar_frames_new = 100;
			pillar_add((sigma_topleft.x + 28), sigma_bullet_top, 1, 0x6F);
			pillar_add((sigma_topleft.x + 92), sigma_bullet_top, 1, 0x6F);
			pillar_frames_new = 16;
		}
		if(sigma_1619C_left >= (PLAYFIELD_RIGHT + BULLET16_W)) {
			sigma_1619C_left = PLAYFIELD_LEFT;
			sigma_1619C_round++;
			if(sigma_1619C_round >= 2) {
				boss_phase_frame = 0;
			}
		}
	}
}

#pragma option -a2

void near sigma_162D3(void)
{
	sigma_pillar_offsets_t offsets = { 60, 44, 76, 28, 92 };
	unsigned char angle;
	int i;

	if(sigma_15EF7()) {
		return;
	}
	if(boss_phase_frame == 100) {
		sigma_pillar_i = 0;
	} else if(boss_phase_frame == 114) {
		pillar_add(
			(offsets.v[sigma_pillar_i++] + sigma_topleft.x), sigma_bullet_top, 1, 0x6F
		);
	} else if(boss_phase_frame == 118) {
		pillar_add(
			(offsets.v[sigma_pillar_i++] + sigma_topleft.x), sigma_bullet_top, 1, 0x6F
		);
		pillar_add(
			(offsets.v[sigma_pillar_i++] + sigma_topleft.x), sigma_bullet_top, 1, 0x6F
		);
	} else if(boss_phase_frame == 122) {
		pillar_add(
			(offsets.v[sigma_pillar_i++] + sigma_topleft.x), sigma_bullet_top, 1, 0x6F
		);
		pillar_add(
			(offsets.v[sigma_pillar_i] + sigma_topleft.x), sigma_bullet_top, 1, 0x6F
		);
		sigma_pillar_i = 0;
	} else if(boss_phase_frame == 242) {
		pillar_add(
			(offsets.v[sigma_pillar_i++] + sigma_topleft.x), sigma_bullet_top, 1, 0x6F
		);
	} else if(boss_phase_frame == 246) {
		pillar_add(
			(offsets.v[sigma_pillar_i++] + sigma_topleft.x), sigma_bullet_top, 1, 0x6F
		);
		pillar_add(
			(offsets.v[sigma_pillar_i++] + sigma_topleft.x), sigma_bullet_top, 1, 0x6F
		);
	} else if(boss_phase_frame == 250) {
		pillar_add(
			(offsets.v[sigma_pillar_i++] + sigma_topleft.x), sigma_bullet_top, 1, 0x6F
		);
		pillar_add(
			(offsets.v[sigma_pillar_i] + sigma_topleft.x), sigma_bullet_top, 1, 0x6F
		);
		sigma_pillar_i = 0;
	} else if(
		(boss_phase_frame > 120) &&
		(boss_phase_frame < 240) &&
		((boss_phase_frame & 0xF) == 0)
	) {
		for((angle = 0x90, i = 0); angle < 0xF0; (angle += 0x10, i++)) {
			bullets_add_16x16(
				sigma_bullet_left,
				sigma_bullet_top,
				angle,
				BSM_BOUNCE_LEFT_RIGHT_TOP_BOTTOM,
				((i % 2) + PAT_BULLET16_BILLIARD_BALL_RED),
				((3 << 4) + 12)
			);
		}
	}
}

#define player_center_x() (player_topleft.x + 16)
#define player_center_y() (player_topleft.y + 16)

void near sigma_16421(void)
{
	int first_failed;

	if(boss_phase_frame < 50) {
		return;
	}
	if(boss_phase_frame == 50) {
		snd_se_play(9);
	}
	if(boss_phase_frame < 100) {
		patnum_2064E = ((page_back * 4) + 128);
		return;
	}
	if(boss_phase_frame == 100) {
		vector2_between_plus(
			sigma_bullet_left,
			sigma_bullet_top,
			player_center_x(),
			player_center_y(),
			0x00,
			sigma_16421_velocity.x,
			sigma_16421_velocity.y,
			48
		);
		sigma_16421_velocity_x_2 = -sigma_16421_velocity.x;
		patnum_2064E = 128;
		sigma_16421_left_1 = sigma_bullet_left;
		sigma_16421_left_2 = sigma_bullet_left;
		sigma_circle_hitbox_shrink = -4;
		sigma_circle_frame_mask = 3;
		sigma_16421_top = sigma_bullet_top;
	}
	if((boss_phase_frame & 7) == 0) {
		first_failed = sigma_circle_add(sigma_16421_left_1, sigma_16421_top, 48);
		if(
			sigma_circle_add(sigma_16421_left_2, sigma_16421_top, 48) &&
			first_failed
		) {
			boss_phase_frame = 0;
		}
		sigma_16421_left_1 += sigma_16421_velocity.x;
		sigma_16421_left_2 += sigma_16421_velocity_x_2;
		sigma_16421_top += sigma_16421_velocity.y;
		if(player_center_x() > sigma_16421_left_1) {
			sigma_16421_left_1 += 16;
		} else if(player_center_x() < sigma_16421_left_1) {
			sigma_16421_left_1 -= 16;
		}
		if(player_center_x() > sigma_16421_left_2) {
			sigma_16421_left_2 -= 16;
		} else if(player_center_x() < sigma_16421_left_2) {
			sigma_16421_left_2 += 16;
		}
		bullets_add_pellet(
			sigma_bullet_left,
			sigma_bullet_top,
			randring2_next8(),
			BG_16_RING,
			((3 << 4) + 12)
		);
	}
}

void near sigma_16555(void)
{
	if(sigma_15EF7()) {
		return;
	}
	if(boss_phase_frame == 50) {
		sigma_circle_frame_mask = 7;
	}
	if((boss_phase_frame & 0x3F) == 32) {
		sigma_circle_add(player_center_x(), player_center_y(), 48);
	}
	if((boss_phase_frame & 0x3F) == 0) {
		bullets_add_pellet(
			sigma_bullet_left,
			sigma_bullet_top,
			randring2_next8(),
			BG_32_RING,
			((3 << 4) + 12)
		);
	}
}

int near sigma_165A5(void)
{
	if(boss_phase_frame < 50) {
		return 1;
	}
	if(boss_phase_frame == 50) {
		sigma_165A5_velocity = (player_left_of_center() ? 2 : -2);
	}
	if(boss_phase_frame < 130) {
		*boss_left_on_back_page += sigma_165A5_velocity;
	} else if(boss_phase_frame < 290) {
		*boss_left_on_back_page -= sigma_165A5_velocity;
	} else if(boss_phase_frame < 370) {
		*boss_left_on_back_page += sigma_165A5_velocity;
	} else {
		boss_phase_frame = 0;
	}
	return 0;
}

void near sigma_16606(void)
{
	if(sigma_165A5()) {
		return;
	}
	if(boss_phase_frame == 50) {
		sigma_circle_frame_mask = 7;
	}
	if((boss_phase_frame & 0x3F) == 0) {
		sigma_circle_add(player_center_x(), player_center_y(), 48);
	}
	if((boss_phase_frame & 7) == 0) {
		bullets_add_pellet(
			sigma_bullet_left,
			sigma_bullet_top,
			0x00,
			BG_2_SPREAD_ULTRAWIDE_AIMED,
			((4 << 4) + 6)
		);
	}
}

void near sigma_16650(void)
{
	if(sigma_165A5()) {
		return;
	}
	if(boss_phase_frame == 50) {
		sigma_16650_angle = 0x00;
	}
	if((boss_phase_frame & 7) == 0) {
		bullets_add_16x16(
			sigma_bullet_left,
			sigma_bullet_top,
			sigma_16650_angle,
			BG_8_RING,
			PAT_BULLET16_BALL,
			(5 << 4)
		);
		sigma_16650_angle += 0x08;
	}
}

void near sigma_1668E(void)
{
	if(sigma_165A5()) {
		return;
	}
	if(boss_phase_frame == 50) {
		sigma_circle_frame_mask = 7;
	}
	if((boss_phase_frame & 0x1F) == 0) {
		sigma_circle_add(player_center_x(), player_center_y(), 48);
	}
	if((boss_phase_frame & 0x1F) == 16) {
		bullets_add_pellet(
			sigma_bullet_left,
			sigma_bullet_top,
			randring2_next8(),
			BG_16_RING,
			((3 << 4) + 12)
		);
	}
}

void sigma_patterns_run(int pattern_count)
{
	switch(sigma_pattern) {
	case 0:	sigma_pattern_funcs[0]();	break;
	case 1:	sigma_pattern_funcs[1]();	break;
	case 2:	sigma_pattern_funcs[2]();	break;
	}
	if(boss_phase_frame == 0) {
		sigma_pattern++;
		if(sigma_pattern >= pattern_count) {
			sigma_pattern = 0;
			sigma_patterns_done = true;
		}
		if(boss_damage >= sigma_phase_damage) {
			boss_damage = 0;
			sigma_phase++;
			sigma_pattern = 0;
			sigma_patterns_done = false;
		}
	}
}

#define sigma_phase_start(f0, f1) { \
	sigma_pattern = 0; \
	boss_phase_frame = 0; \
	sigma_phase++; \
	sigma_patterns_done = false; \
	sigma_pattern_funcs[0] = f0; \
	sigma_pattern_funcs[1] = f1; \
}

int sigma_update(void)
{
	unsigned char angle;
	register int i;

	sigma_topleft.x = *boss_left_on_back_page;
	sigma_topleft.y = *boss_top_on_back_page;
	sigma_bullet_left = (sigma_topleft.x + 60);
	sigma_bullet_top = (sigma_topleft.y + 60);
	boss_phase_frame++;
	shot_target_left = sigma_bullet_left;
	shot_target_top = sigma_bullet_top;
	if(!(stage_frame & 1)) {
		sigma_ring_angle += 8;
		grcg_setcolor(GC_RMW, 4);
		for((i = 0, angle = sigma_ring_angle); i < 2; (i++, angle += 0x80)) {
			circle_dots_put(224, 200, angle, ((reduce_effects * 14) + 2));
		}
		grcg_off();
	}
	if(byte_2066A) {
		if(sigma_defeat_animate()) {
			return 2;
		}
		goto circles;
	}
	if(sigma_phase == 0) {
		if(boss_phase_frame > 50) {
			sigma_phase_start(sigma_15D56, sigma_15E84);
			sigma_pattern_funcs[2] = sigma_15F6F;
			sigma_phase_damage = 1800;
		}
	} else if(sigma_phase == 1) {
		sigma_patterns_run(3);
	} else if(sigma_phase == 2) {
		sigma_phase_start(sigma_15F95, sigma_16176);
		sigma_phase_damage = 1800;
	} else if(sigma_phase == 3) {
		sigma_patterns_run(2);
	} else if(sigma_phase == 4) {
		sigma_phase_start(sigma_1619C, sigma_162D3);
		sigma_phase_damage = 1800;
	} else if(sigma_phase == 5) {
		sigma_patterns_run(2);
	} else if(sigma_phase == 6) {
		sigma_phase_start(sigma_16421, sigma_16555);
		sigma_phase_damage = 1800;
	} else if(sigma_phase == 7) {
		sigma_patterns_run(2);
	} else if(sigma_phase == 8) {
		sigma_phase_start(sigma_16606, sigma_16650);
		sigma_pattern_funcs[2] = sigma_1668E;
		sigma_phase_damage = 5000;
	} else if(sigma_phase == 9) {
		sigma_patterns_run(3);
		if(boss_damage >= 1300) {
			byte_2066A = true;
			score += 300000;
			boss_phase_frame = 0;
		}
	}
	sigma_topleft.x = *boss_left_on_back_page;
	sigma_topleft.y = *boss_top_on_back_page;
	sigma_hittest_and_put();
	sigma_frame++;

circles:
	sigma_circles_update_and_render();
	return 1;
}

void sigma_init(void)
{
	sub_1A6C5();
	tile_mode = TM_NONE;
	dialog_pre();
	super_clean(128, 192);
	super_patnum = 128;
	super_entry_bfnt("stage5b1.bft");
	super_entry_bfnt("stage5b2.bft");
	grc_setclip(PLAYFIELD_LEFT, 0, PLAYFIELD_RIGHT, (RES_Y - 1));
	dialog_script_extra_pre_intro_animate();
	boss_left_on_page[0] = (PLAYFIELD_LEFT + (PLAYFIELD_W / 2) - 64);
	boss_left_on_page[1] = (PLAYFIELD_LEFT + (PLAYFIELD_W / 2) - 64);
	sigma_topleft.x = (PLAYFIELD_LEFT + (PLAYFIELD_W / 2) - 64);
	boss_top_on_page[0] = (PLAYFIELD_TOP + 32);
	boss_top_on_page[1] = (PLAYFIELD_TOP + 32);
	sigma_topleft.y = (PLAYFIELD_TOP + 32);
	boss_damage = 0;
	boss_phase_frame = 0;
	patnum_2064E = 128;
	sigma_phase = 0;
	sigma_circle_frame_mask = 7;
	bullets_clear();
	shots_clear();
	palette_white_out(1);
	sigma_put();
	sub_13ABB("boss5.m");
	palette_white_in(1);
	dialog_script_generic_part_animate(DS_PREBOSS);
	dialog_post();
}

void sigma_end(void)
{
	dialog_pre();
	dialog_script_generic_part_animate(DS_POSTBOSS);
	stage_extra_clear_bonus_animate();
	key_delay();
	resident->stage = 0x7F;
	resident->score = ((score * 10) + resident->continues_used);
	scoredat_extra_clear_save();
	GameExecl("maine");
}
