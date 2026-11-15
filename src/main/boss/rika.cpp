/* ReC98
 * -----
 * Stage 1: background particles, midboss and the boss, Rika. Also contains
 * the boss BGM loader.
 */

#pragma option -zCmain_03_TEXT -zPmain_03 -G

#include "platform.h"
#include "x86real.h"
#include "pc98.h"
#include "libs/master.lib/master.hpp"
#include "libs/master.lib/pc98_gfx.hpp"
#include "th01/rank.h"
#include "th02/v_colors.hpp"
#include "th02/resident.hpp"
#include "th02/core/globals.hpp"
#include "th02/hardware/input.hpp"
#include "th02/hardware/pages.hpp"
#include "th02/math/randring.hpp"
#include "th02/snd/snd.h"
#include "th02/main/circle.hpp"
#include "th02/main/frames.hpp"
#include "th02/main/playfld.hpp"
#include "th02/main/score.hpp"
#include "th02/main/scroll.hpp"
#include "th02/main/spark.hpp"
#include "th02/main/boss/boss.hpp"
#include "th02/main/bullet/bullet.hpp"
#include "th02/main/dialog/dialog.hpp"
#include "th02/main/hud/overlay.hpp"
#include "th02/main/player/player.hpp"
#include "th02/main/stage/bonus.hpp"
#include "th02/main/stage/stage.hpp"
#include "th02/main/tile/tile.hpp"

extern screen_x_t shot_target_left;
extern screen_y_t shot_target_top;
extern int patnum_2064E;
extern uint8_t byte_2066A;
extern uint8_t byte_2066E;
extern uint8_t byte_2066F;
extern uint8_t group_20670;
extern uint8_t byte_20671;
extern uint8_t byte_20672;
extern unsigned char angle_1E510;
extern uint8_t pillar_frames_new;
extern unsigned int stage2_lightning_frame;
extern int tile_rows_scrolled;

int pascal near shots_hittest(
	screen_x_t left, screen_y_t top, pixel_t w, pixel_t h
);
void pascal near pillar_add(
	screen_x_t left, screen_y_t top, int duration, int patnum
);
void pascal near sub_FFF8(screen_x_t center_x, screen_y_t center_y, int frame);
void near stage2_lightning_update(void);
void sub_129FC(void);
void near dialog_pre(void);
void near dialog_post(void);

// Background
// ----------

struct stage1_particle_t {
	uint8_t flag;
	int8_t padding;
	screen_x_t left[PAGE_COUNT];
	screen_y_t top[PAGE_COUNT];
	pixel_t speed;
};

static const int STAGE1_PARTICLE_COUNT = 50;

// Uninitialized data, in the original order
#pragma option -a2
screen_point_t rika_topleft;
int rika_phase;
bool16 midboss1_active;
int midboss1_patnum;
stage1_particle_t stage1_particles[STAGE1_PARTICLE_COUNT];
unsigned char rika_angle;
unsigned int rika_frame;
#pragma option -a-

int stage1_bg_frame = 0;

void pascal near sub_134B6(screen_x_t left, screen_y_t top)
{
	stage1_particle_t *p;
	register int i;

	p = stage1_particles;
	for(i = 0; i < STAGE1_PARTICLE_COUNT; (i++, p++)) {
		if(p->flag != 0) {
			continue;
		}
		p->flag = 1;
		p->left[0] = left;
		p->top[0] = top;
		p->left[1] = left;
		p->top[1] = top;
		p->speed = (randring2_next8_and(3) + 3);
		return;
	}
}

void sub_13513(void)
{
	stage1_particle_t *p;
	register int i;

	p = stage1_particles;
	for(i = 0; i < STAGE1_PARTICLE_COUNT; (i++, p++)) {
		if(p->flag == 0) {
			continue;
		}
		tiles_invalidate_rect(p->left[page_back], p->top[page_back], 2, 3);
		if(p->flag == 2) {
			p->flag = 0;
			continue;
		}
		p->left[page_back] = p->left[page_front];
		p->top[page_back] = p->top[page_front];
	}
}

void near sub_135BB(void)
{
	int i;
	stage1_particle_t *p;
	register screen_x_t left;
	register vram_y_t top;

	p = stage1_particles;
	for(i = 0; i < STAGE1_PARTICLE_COUNT; (i++, p++)) {
		if(p->flag != 1) {
			continue;
		}
		p->left[page_back] -= p->speed;
		p->top[page_back] += p->speed;
		left = p->left[page_back];
		top = p->top[page_back];
		if(left < 32) {
			p->flag = 2;
			continue;
		}
		if((top < 384) && (left < 416)) {
			top += scroll_line;
			if(top >= RES_Y) {
				top -= RES_Y;
			}
			circle_dot_left = left;
			circle_dot_top = top;
			circle_dot_put(2);
		}
	}
	grcg_off();
}

void sub_13671(void)
{
	RGB8 col;

	if((stage1_bg_frame < 999) && (stage1_bg_frame != 0)) {
		stage1_bg_frame++;
		col.c.g = ((stage1_bg_frame / 16) + 144);
		col.c.b = ((stage1_bg_frame / 8) + 144);
		Palettes[8].c.r = 176;
		Palettes[8].c.g = col.c.g;
		Palettes[8].c.b = col.c.b;
		col.c.r = (160 - (stage1_bg_frame / 16));
		col.c.g = (160 - (stage1_bg_frame / 16));
		col.c.b = (255 - (stage1_bg_frame / 4));
		Palettes[13].c.r = col.c.r;
		Palettes[13].c.g = col.c.g;
		Palettes[13].c.b = col.c.b;
		if(col.c.r <= 0x80) {
			stage1_bg_frame = 1000;
			grc_setclip(PLAYFIELD_LEFT, 0, PLAYFIELD_RIGHT, (RES_Y - 1));
			Palettes[14].c.r = 224;
			Palettes[14].c.g = 192;
			Palettes[14].c.b = 176;
			spark_accel_x.v = -1;
			stage2_lightning_frame = 1;
		}
		palette_show();
		return;
	}
	if(stage1_bg_frame >= 1000) {
		stage2_lightning_update();
		stage1_bg_frame++;
		if(((reduce_effects * 2) + 1002) < stage1_bg_frame) {
			sub_134B6(((randring2_next16() % 640) + 48), 16);
			stage1_bg_frame = 1000;
		}
		egc_off();
		grcg_setcolor(GC_RMW, 8);
		sub_135BB();
	}
}

// Midboss
// -------
int midboss1_kill_frame = 0;

int midboss1_invalidate(void)
{
	boss_left_on_back_page = &boss_left_on_page[page_back];
	boss_top_on_back_page = &boss_top_on_page[page_back];
	if(byte_2066A) {
		tiles_invalidate_rect(
			(*boss_left_on_back_page - 16), *boss_top_on_back_page, 96, 96
		);
	} else {
		tiles_invalidate_rect(
			*boss_left_on_back_page, *boss_top_on_back_page, 64, 96
		);
	}
	*boss_left_on_back_page = boss_left_on_page[page_front];
	*boss_top_on_back_page = boss_top_on_page[page_front];
	return midboss1_active;
}

void near midboss1_kill_animate(void)
{
	register vram_y_t vram_top;
	register int zoom;

	zoom = 10;
	zoom += (midboss1_kill_frame >> 3);
	midboss1_kill_frame++;
	sparks_add(
		(*boss_left_on_back_page + 32),
		(*boss_top_on_back_page + 48),
		((3 << 4) + 12),
		1,
		false
	);
	if(midboss1_kill_frame >= 64) {
		sparks_add(
			(*boss_left_on_back_page + 32),
			(*boss_top_on_back_page + 48),
			(8 << 4),
			48,
			true
		);
		midboss1_kill_frame = 0;
		boss_phase_frame = 0;
		shot_target_left = -1;
		shot_target_top = -1;
		midboss1_active = false;
	} else {
		if((midboss1_kill_frame & 3) == 0) {
			(*boss_top_on_back_page)++;
		}
		vram_top = *boss_top_on_back_page;
		vram_top += scroll_line;
		if(vram_top >= RES_Y) {
			vram_top -= RES_Y;
		}
		super_zoom((*boss_left_on_back_page - 16), vram_top, zoom, 3);
	}
}

void near midboss1_fire(void)
{
	register screen_y_t top;

	top = (*boss_top_on_back_page + 88);
	if((boss_phase_frame & 0x7F) == 0) {
		bullets_add_pellet(
			(*boss_left_on_back_page + 28),
			top,
			0x00,
			BG_4_SPREAD_WIDE_AIMED,
			((3 << 4) + 2)
		);
	} else if((boss_phase_frame & 0x7F) == 48) {
		bullets_add_pellet(
			(*boss_left_on_back_page + 28),
			top,
			0x00,
			BG_3_SPREAD_MEDIUM_AIMED,
			((1 << 4) + 14)
		);
	} else if((boss_phase_frame & 0x7F) == 96) {
		bullets_add_pellet(
			(*boss_left_on_back_page + 28),
			top,
			0x00,
			BG_4_SPREAD_WIDE_AIMED,
			((3 << 4) + 2)
		);
	}
}

void midboss1_update_and_render(void)
{
	int damage;
	register vram_y_t vram_top;

	shot_target_left = 216;
	shot_target_top = 64;
	boss_phase_frame++;
	if(boss_phase_frame == 1) {
		boss_left_on_page[0] = (PLAYFIELD_LEFT + (PLAYFIELD_W / 2) - 32);
		boss_left_on_page[1] = (PLAYFIELD_LEFT + (PLAYFIELD_W / 2) - 32);
		boss_top_on_page[0] = (PLAYFIELD_TOP - 32);
		boss_top_on_page[1] = (PLAYFIELD_TOP - 32);
		boss_damage = 0;
		midboss1_patnum = 148;
		midboss1_active = true;
		stage1_bg_frame = 1;
		return;
	}
	if(boss_phase_frame < 18) {
		*boss_top_on_back_page += 2;
		vram_top = *boss_top_on_back_page;
		vram_top += scroll_line;
		if(vram_top < 0) {
			vram_top += RES_Y;
		} else if(vram_top >= RES_Y) {
			vram_top -= RES_Y;
		}
		goto render;
	}
	if(scroll_line & 1) {
		midboss1_patnum = 149;
	} else {
		midboss1_patnum = 148;
	}
	if(byte_2066A) {
		midboss1_kill_animate();
		return;
	}
	midboss1_fire();
	if((scroll_line & 0xF) == 0) {
		vram_top = 48;
		vram_top += scroll_line;
		if(vram_top >= RES_Y) {
			vram_top -= RES_Y;
		}
		tile_ring_set_and_put_both_8(*boss_left_on_back_page, vram_top, 60);
		tile_ring_set_and_put_both_8((*boss_left_on_back_page + 16), vram_top, 61);
		tile_ring_set_and_put_both_8((*boss_left_on_back_page + 32), vram_top, 62);
		tile_ring_set_and_put_both_8((*boss_left_on_back_page + 48), vram_top, 63);
	}
	vram_top = *boss_top_on_back_page;
	vram_top += scroll_line;
	if(vram_top >= RES_Y) {
		vram_top -= RES_Y;
	}
	if((damage = shots_hittest(
		*boss_left_on_back_page, *boss_top_on_back_page, 64, 96
	)) != 0) {
		boss_damage += damage;
		if((boss_damage <= 300) || (vram_top >= 304)) {
			snd_se_play(4);
			super_roll_put_1plane(
				*boss_left_on_back_page, vram_top, 148, 0, super_plane(V_WHITE)
			);
		} else {
			snd_se_play(2);
			byte_2066A = true;
			score_delta += 10000;
		}
		return;
	}
	if((tile_rows_scrolled >= 184) && (*boss_top_on_back_page <= 304)) {
		snd_se_play(2);
		byte_2066A = true;
		return;
	}
render:
	super_roll_put(*boss_left_on_back_page, vram_top, midboss1_patnum);
}

// Boss BGM
// --------

void sub_13ABB(char far *fn)
{
	snd_kaja_func(KAJA_SONG_STOP, 0);
	snd_load(fn, SND_LOAD_SONG);
	snd_kaja_func(KAJA_SONG_PLAY, 0);
}

// Boss
// ----
int rika_kill_frame = 0;

void rika_init(void)
{
	sub_13ABB("boss1.m");
	dialog_pre();
	dialog_script_generic_part_animate(DS_PREBOSS);
	dialog_post();
	angle_1E510 = 0x20;
	boss_left_on_page[0] = (PLAYFIELD_LEFT + (PLAYFIELD_W / 2) - 32);
	boss_left_on_page[1] = (PLAYFIELD_LEFT + (PLAYFIELD_W / 2) - 32);
	boss_damage = 0;
	byte_2066A = false;
	patnum_2064E = 150;
	stage_frame = 0;
	rika_phase = 0;
	sub_129FC();
	patnum_2064E = 150;
	rika_topleft.y = 48;
	rika_topleft.y += scroll_line;
	if(rika_topleft.y >= RES_Y) {
		rika_topleft.y -= RES_Y;
	}
	if(rank != RANK_EASY) {
		byte_2066E = 0x0B;
		byte_2066F = 0x15;
		group_20670 = BG_1;
		byte_20671 = 0x02;
		byte_20672 = 3;
	} else {
		byte_2066E = 0x19;
		byte_2066F = 0x15;
		group_20670 = BG_1;
		byte_20671 = 0x20;
		byte_20672 = 7;
	}
}

void rika_end(void)
{
	dialog_pre();
	dialog_script_generic_part_animate(DS_POSTBOSS);
	stage_clear_bonus_animate();
	key_delay();
	overlay_stage_leave_animate();
	stage_id++;
	spark_accel_x.v = 0;
	stage2_lightning_frame = 0;
}

void rika_bg_render(void)
{
	boss_left_on_back_page = &boss_left_on_page[page_back];
	tiles_invalidate_rect(*boss_left_on_back_page, 48, 64, 96);
	*boss_left_on_back_page = boss_left_on_page[page_front];
	if((rika_phase == 0) && (boss_phase_frame < 3)) {
		tiles_invalidate_rect(208, 16, 32, 32);
	}
}

int near rika_defeat_animate(void)
{
	register int zoom;

	sparks_add((rika_topleft.x + 32), 128, ((3 << 4) + 2), 2, false);
	zoom = 10;
	zoom += ((rika_kill_frame - 32) / 8);
	rika_kill_frame++;
	if(rika_kill_frame >= 80) {
		rika_kill_frame = 0;
		return 1;
	}
	if(boss_phase_frame < 32) {
		super_roll_put(rika_topleft.x, rika_topleft.y, patnum_2064E);
	} else {
		super_zoom(rika_topleft.x, (rika_topleft.y + 16), zoom, 2);
	}
	sub_FFF8((rika_topleft.x + 24), 112, rika_kill_frame);
	return 0;
}

void near rika_fire(void)
{
	uint8_t group;
	register screen_x_t left;

	left = (rika_topleft.x + 28);
	if(boss_damage < 700) {
		rika_phase = 0;
		if((boss_phase_frame & 0x3F) == 0) {
			snd_se_play(3);
			bullets_add_pellet(
				left, (PLAYFIELD_TOP + 96), 0x00, byte_2066E, ((3 << 4) + 2)
			);
			bullets_add_pellet(
				left, (PLAYFIELD_TOP + 96), 0x40, byte_2066F, ((1 << 4) + 14)
			);
		}
	} else if(boss_damage < 1400) {
		if((stage_frame & 0x1F) == 0) {
			rika_angle += 0x18;
			rika_angle %= 0x80;
			left = (rika_topleft.x + 8);
			snd_se_play(3);
			bullets_add_pellet(
				left, (PLAYFIELD_TOP + 96), rika_angle, group_20670, ((3 << 4) + 6)
			);
			bullets_add_pellet(
				left, (PLAYFIELD_TOP + 96), rika_angle, byte_20671, ((2 << 4) + 12)
			);
			left += 44;
			bullets_add_pellet(
				left,
				(PLAYFIELD_TOP + 96),
				(0x80 - rika_angle),
				group_20670,
				((3 << 4) + 6)
			);
			bullets_add_pellet(
				left,
				(PLAYFIELD_TOP + 96),
				(0x80 - rika_angle),
				byte_20671,
				((2 << 4) + 12)
			);
		}
	} else {
		if(rika_phase == 0) {
			rika_phase = 1;
			rika_frame = 0;
			rika_angle = 0x00;
		}
		rika_frame++;
		if(rika_frame < 100) {
			if((rika_frame & 7) < 4) {
				patnum_2064E = 150;
			} else {
				patnum_2064E = 151;
			}
			if((rika_frame & 0x1F) == 0) {
				snd_se_play(9);
			}
		} else if(rika_frame == 100) {
			if((rika_frame & 0x1F) == 0) {
				snd_se_play(9);
			}
			pillar_frames_new = 64;
			pillar_add((rika_topleft.x + 2), 112, 160, 0x6F);
			pillar_add((rika_topleft.x + 46), 112, 160, 0x6F);
		} else if((rika_frame > 164) && (rika_frame < 280)) {
			if(rika_frame < 250) {
				if((rika_frame & 7) < 4) {
					patnum_2064E = 150;
				} else {
					patnum_2064E = 151;
				}
			} else if(rika_frame == 260) {
				patnum_2064E = 152;
			} else if(rika_frame == 270) {
				patnum_2064E = 153;
			}
			if((rika_frame & 0xF) == 0) {
				snd_se_play(6);
			}
			if((rika_frame & 0xF) == 0) {
				if((rika_frame & 0x3F) == 0) {
					group = BG_4_SPREAD_WIDE;
				} else if((rika_frame & 0x3F) == 16) {
					group = BG_4_SPREAD_MEDIUM;
				} else if((rika_frame & 0x3F) == 32) {
					group = BG_4_SPREAD_WIDE;
				} else if((rika_frame & 0x3F) == 48) {
					group = BG_4_SPREAD_MEDIUM;
				}
				bullets_add_pellet(
					left, (PLAYFIELD_TOP + 96), 0x40, group, ((3 << 4) + 10)
				);
			}
		} else if((rika_frame >= 280) && ((rika_frame & byte_20672) == 0)) {
			bullets_add_16x16(
				left,
				(PLAYFIELD_TOP + 56),
				0x01,
				BG_RANDOM_ANGLE_AND_SPEED,
				PAT_BULLET16_OUTLINED_BALL_BEIGE,
				((2 << 4) + 12)
			);
		}
	}
}

int rika_hittest(void)
{
	int damage;

	if((damage = shots_hittest(rika_topleft.x, 48, 64, 96)) != 0) {
		boss_damage += damage;
		if(boss_damage <= 2260) {
			snd_se_play(4);
			super_roll_put_1plane(
				rika_topleft.x,
				rika_topleft.y,
				patnum_2064E,
				0,
				super_plane(V_WHITE)
			);
		} else {
			byte_2066A = true;
			score_delta += 20000;
			player_invincibility_time = BOSS_DEFEAT_INVINCIBILITY_FRAMES;
		}
		return 1;
	}
	return 0;
}

void rika_move(void)
{
	unsigned char angle;

	if(rika_phase == 0) {
		if(boss_phase_frame < 16) {
			if((boss_phase_frame & 3) == 0) {
				if(rank >= RANK_HARD) {
					angle = boss_phase_frame;
				} else {
					angle = 0x00;
				}
				bullets_add_pellet(
					228,
					112,
					angle,
					BG_16_RING,
					((boss_phase_frame * 4) + ((2 << 4) + 8))
				);
			}
		} else if(boss_phase_frame < 116) {
			(*boss_left_on_back_page)--;
		} else if(boss_phase_frame < 316) {
			(*boss_left_on_back_page)++;
		} else {
			if(boss_phase_frame >= 416) {
				boss_phase_frame = 16; // Skip the initial 16-ring subpattern
			}
			(*boss_left_on_back_page)--;
		}
	} else if(rika_phase == 1) {
		if(*boss_left_on_back_page < (PLAYFIELD_LEFT + (PLAYFIELD_W / 2) - 32)) {
			(*boss_left_on_back_page)++;
		} else if(boss_phase_frame > 192) {
			(*boss_left_on_back_page)--;
		} else {
			rika_phase = 2;
		}
	} else if(rika_phase == 2) {
		boss_phase_frame = 0;
	}
}

int rika_update(void)
{
	shot_target_left = (rika_topleft.x + 24);
	shot_target_top = 48;
	boss_phase_frame++;
	if(!byte_2066A) {
		rika_move();
		rika_topleft.x = *boss_left_on_back_page;
		if(!rika_hittest()) {
			super_roll_put(rika_topleft.x, rika_topleft.y, patnum_2064E);
		}
		rika_fire();
	} else if(rika_defeat_animate()) {
		return 2;
	}
	return 1;
}
