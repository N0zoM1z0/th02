/* ReC98
 * -----
 * Stage 2: background, midboss and the boss, Meira
 */

#pragma option -zCmain_03_TEXT -zPmain_03 -G -a2

#include "platform.h"
#include "x86real.h"
#include "pc98.h"
#include "libs/master.lib/master.hpp"
#include "libs/master.lib/pc98_gfx.hpp"
#include "th01/rank.h"
#include "th02/v_colors.hpp"
#include "th02/core/globals.hpp"
#include "th02/hardware/input.hpp"
#include "th02/hardware/pages.hpp"
#include "th02/math/randring.hpp"
#include "th02/snd/snd.h"
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
extern unsigned char angle_1E510;

int pascal near shots_hittest(
	screen_x_t left, screen_y_t top, pixel_t w, pixel_t h
);
void pascal near sub_FFF8(screen_x_t center_x, screen_y_t center_y, int frame);
void sub_13ABB(char far *fn);
void near dialog_pre(void);
void near dialog_post(void);

// State
// -----

struct meira_flower_t {
	uint8_t flag;
	uint8_t group;
	screen_x_t left;
	screen_y_t top;
	unsigned char angle;
	uint8_t frame;
	uint8_t speed;
	uint8_t patnum;
	uint8_t frames_left;
	int8_t padding;
};

static const int MEIRA_FLOWER_COUNT = 40;
static const int MEIRA_TRAIL_COUNT = 3;
extern int meira_kill_frame;
// -----

// Background
// ----------

// Uninitialized data, in the original order
bool16 midboss2_active;
unsigned int stage2_lightning_next;
screen_x_t meira_trail_left[PAGE_COUNT][MEIRA_TRAIL_COUNT];
screen_y_t meira_trail_top[PAGE_COUNT][MEIRA_TRAIL_COUNT];
bool16 meira_trail_active;
meira_flower_t meira_flowers[MEIRA_FLOWER_COUNT];
uint8_t meira_flower_patnum;
uint8_t meira_flower_fire_i;
uint8_t meira_flower_group;
uint8_t meira_flower_speed;
int meira_flower_frames;
uint8_t meira_dir;
subpixel_t meira_speed_1;
subpixel_t meira_speed_2;
screen_x_t meira_origin_left;
screen_y_t meira_origin_top;
screen_x_t meira_target_left;
screen_y_t meira_target_top;
int meira_step;
uint8_t meira_phase;
uint8_t meira_pattern;

unsigned int stage2_bg_frame = 0;
unsigned int stage2_lightning_frame = 0;
bool stage2_bg_clip_reset = true;
int midboss2_kill_frame = 0;
int meira_kill_frame = 0;

void pascal near sub_134B6(screen_x_t left, screen_y_t top);
void near sub_135BB(void);

void near stage2_lightning_update(void)
{
	if(stage2_lightning_frame == 0) {
		return;
	}
	if(stage2_lightning_frame == 1) {
		stage2_lightning_next = 100;
	}
	stage2_lightning_frame++;
	if(
		(stage2_lightning_frame >= stage2_lightning_next) &&
		((stage2_lightning_next + 8) > stage2_lightning_frame)
	) {
		if(stage2_lightning_frame & 1) {
			PaletteTone = 140;
		} else {
			PaletteTone = 100;
		}
		palette_show();
		if((stage2_lightning_next + 7) == stage2_lightning_frame) {
			PaletteTone = 100;
			palette_show();
			stage2_lightning_next += ((randring2_next16() & 0x3FF) + 20);
		}
	}
}

void sub_140AE(void)
{
	if(stage2_bg_clip_reset) {
		grc_setclip(PLAYFIELD_LEFT, 0, PLAYFIELD_RIGHT, (RES_Y - 1));
		stage2_bg_clip_reset = false;
		spark_accel_x.v = -1;
	}
	stage2_bg_frame++;
	if(!(stage2_bg_frame & 1)) {
		sub_134B6(((randring2_next16() % 640) + 48), 16);
	}
	stage2_lightning_update();
	egc_off();
	grcg_setcolor(GC_RMW, 3);
	sub_135BB();
}

// Midboss
// -------

int midboss2_invalidate(void)
{
	boss_left_on_back_page = &boss_left_on_page[page_back];
	boss_top_on_back_page = &boss_top_on_page[page_back];
	tiles_invalidate_rect(*boss_left_on_back_page, *boss_top_on_back_page, 64, 64);
	*boss_left_on_back_page = boss_left_on_page[page_front];
	*boss_top_on_back_page = boss_top_on_page[page_front];
	return midboss2_active;
}

void near midboss2_kill_animate(void)
{
	register vram_y_t vram_top;
	register int zoom;

	if((midboss2_kill_frame < 48) && ((midboss2_kill_frame & 0xF) == 5)) {
		sparks_add(
			(*boss_left_on_back_page + 32),
			(*boss_top_on_back_page + 32),
			(8 << 4),
			24,
			true
		);
	}
	zoom = 10;
	zoom += (midboss2_kill_frame >> 3);
	midboss2_kill_frame++;
	if(midboss2_kill_frame >= 64) {
		midboss2_kill_frame = 0;
		boss_phase_frame = 0;
		byte_2066A = false;
		midboss2_active = false;
		shot_target_left = -1;
		shot_target_top = -1;
	} else {
		vram_top = *boss_top_on_back_page;
		vram_top += scroll_line;
		if(vram_top >= RES_Y) {
			vram_top -= RES_Y;
		}
		super_zoom(*boss_left_on_back_page, vram_top, zoom, 2);
	}
}

void near midboss2_fire(void)
{
	if(((boss_phase_frame % 8) == 0) && ((boss_phase_frame % 192) < 64)) {
		bullets_add_16x16(
			(*boss_left_on_back_page + 28),
			(*boss_top_on_back_page + 32),
			(boss_phase_frame << 2),
			BSM_CHASE,
			PAT_BULLET16_NOROI,
			((1 << 4) + 14)
		);
	}
}

void midboss2_update_and_render(void)
{
	int cel;
	int damage;
	register vram_y_t vram_top;
	register int patnum = 0;

	shot_target_left = 216;
	shot_target_top = (*boss_top_on_back_page + 24);
	boss_phase_frame++;
	if(boss_phase_frame == 1) {
		boss_left_on_page[0] = (PLAYFIELD_LEFT + (PLAYFIELD_W / 2) - 32);
		boss_left_on_page[1] = (PLAYFIELD_LEFT + (PLAYFIELD_W / 2) - 32);
		boss_top_on_page[0] = (PLAYFIELD_TOP - 32);
		boss_top_on_page[1] = (PLAYFIELD_TOP - 32);
		boss_damage = 0;
		midboss2_active = true;
		stage2_lightning_frame = 1;
		return;
	} else if(boss_phase_frame < 320) {
		cel = ((scroll_line >> 1) & 3);
		patnum = (cel + 137);
		if((boss_phase_frame & 3) == 0) {
			(*boss_top_on_back_page)++;
		}
		vram_top = *boss_top_on_back_page;
		vram_top += scroll_line;
		if(vram_top < 0) {
			vram_top += RES_Y;
		} else if(vram_top >= RES_Y) {
			vram_top -= RES_Y;
		}
		if(shots_hittest(*boss_left_on_back_page, *boss_top_on_back_page, 64, 58)) {
			snd_se_play(4);
			super_roll_put_1plane(
				*boss_left_on_back_page, vram_top, 136, 0, super_plane(V_WHITE)
			);
			boss_damage++;
			return;
		}
	} else {
		cel = ((scroll_line >> 1) & 3);
		patnum = (cel + 137);
		if(byte_2066A) {
			midboss2_kill_animate();
			return;
		}
		midboss2_fire();
		vram_top = *boss_top_on_back_page;
		vram_top += scroll_line;
		if(vram_top < 0) {
			vram_top += RES_Y;
		} else if(vram_top >= RES_Y) {
			vram_top -= RES_Y;
		}
		if((damage = shots_hittest(
			*boss_left_on_back_page, *boss_top_on_back_page, 64, 58
		)) != 0) {
			boss_damage += damage;
			if((boss_damage <= 380) || (vram_top >= 304)) {
				snd_se_play(4);
				super_roll_put_1plane(
					*boss_left_on_back_page, vram_top, 136, 0, super_plane(V_WHITE)
				);
			} else {
				snd_se_play(2);
				byte_2066A = true;
				score_delta += 20000;
			}
			return;
		}
		if(boss_phase_frame > 1600) {
			byte_2066A = true;
		}
	}
	super_roll_put(*boss_left_on_back_page, vram_top, patnum);
}

// Boss
// ----

void near meira_flowers_invalidate(void);

void meira_bg_render(void)
{
	register int i;

	boss_left_on_back_page = &boss_left_on_page[page_back];
	boss_top_on_back_page = &boss_top_on_page[page_back];
	tiles_invalidate_rect(*boss_left_on_back_page, *boss_top_on_back_page, 64, 64);
	if(meira_trail_active) {
		for(i = 0; i < MEIRA_TRAIL_COUNT; i++) {
			if(
				(meira_trail_left[page_back][i] != *boss_left_on_back_page) &&
				(meira_trail_top[page_back][i] != *boss_top_on_back_page)
			) {
				tiles_invalidate_rect(
					meira_trail_left[page_back][i],
					meira_trail_top[page_back][i],
					64,
					64
				);
			}
			meira_trail_left[page_back][i] = meira_trail_left[page_front][i];
			meira_trail_top[page_back][i] = meira_trail_top[page_front][i];
		}
	}
	*boss_left_on_back_page = boss_left_on_page[page_front];
	*boss_top_on_back_page = boss_top_on_page[page_front];
	meira_flowers_invalidate();
}

int near meira_defeat_animate(void)
{
	register vram_y_t vram_top;
	register int zoom;

	sub_FFF8(
		(*boss_left_on_back_page + 24),
		(*boss_top_on_back_page + 24),
		meira_kill_frame
	);
	sub_FFF8(
		(*boss_left_on_back_page + 24),
		(*boss_top_on_back_page + 24),
		(meira_kill_frame - 24)
	);
	sparks_add(
		(*boss_left_on_back_page + 48),
		(*boss_top_on_back_page + 48),
		((3 << 4) + 12),
		1,
		false
	);
	zoom = 10;
	zoom += ((meira_kill_frame - 32) / 10);
	meira_kill_frame++;
	if(meira_kill_frame >= 96) {
		meira_kill_frame = 0;
		return 1;
	}
	vram_top = *boss_top_on_back_page;
	vram_top += scroll_line;
	if(vram_top >= RES_Y) {
		vram_top -= RES_Y;
	}
	if(boss_phase_frame < 32) {
		super_roll_put(*boss_left_on_back_page, vram_top, patnum_2064E);
	} else {
		super_zoom(*boss_left_on_back_page, vram_top, zoom, 2);
	}
	return 0;
}

void near meira_hittest_and_render(void)
{
	int damage;
	register screen_y_t top;

	top = *boss_top_on_back_page;
	if(
		(*boss_left_on_back_page <= player_topleft.x) &&
		((*boss_left_on_back_page + 32) > player_topleft.x) &&
		((top - 16) < player_topleft.y) &&
		((top + 32) > player_topleft.y)
	) {
		player_is_hit = true;
	}
	top += scroll_line;
	if(top >= RES_Y) {
		top -= RES_Y;
	}
	if((damage = shots_hittest(
		(*boss_left_on_back_page + 8), *boss_top_on_back_page, 48, 48
	)) != 0) {
		boss_damage += damage;
		snd_se_play(4);
		super_roll_put_1plane(
			*boss_left_on_back_page, top, patnum_2064E, 0, super_plane(V_WHITE)
		);
		if(boss_damage >= 2400) {
			byte_2066A = true;
			score_delta += 30000;
			player_invincibility_time = BOSS_DEFEAT_INVINCIBILITY_FRAMES;
		}
	} else {
		super_roll_put(*boss_left_on_back_page, top, patnum_2064E);
	}
}

void pascal near meira_flower_add(
	screen_x_t left,
	screen_y_t top,
	unsigned char angle,
	uint8_t group,
	uint8_t speed,
	uint8_t frames
)
{
	register meira_flower_t near *p = meira_flowers;
	int i;

	for(i = 0; i < MEIRA_FLOWER_COUNT; (i++, p++)) {
		if(p->flag == 0) {
			p->flag = 1;
			p->group = group;
			p->left = left;
			p->top = top;
			p->angle = angle;
			p->frame = 0;
			p->speed = speed;
			p->patnum = meira_flower_patnum;
			p->frames_left = frames;
			return;
		}
	}
}

void near meira_flowers_invalidate(void)
{
	register meira_flower_t near *p = meira_flowers;
	register int i;

	for(i = 0; i < MEIRA_FLOWER_COUNT; (i++, p++)) {
		if(p->flag == 0) {
			continue;
		}
		tiles_invalidate_rect(p->left, p->top, 32, 32);
		if(p->flag == 2) {
			p->flag = 0;
		}
	}
}

void near meira_flowers_update_and_render(void)
{
	int i;
	int patnum;
	register meira_flower_t near *p = meira_flowers;
	register vram_y_t vram_top;

	for(i = 0; i < MEIRA_FLOWER_COUNT; (i++, p++)) {
		if(p->flag != 1) {
			continue;
		}
		if(p->frames_left > 0) {
			vram_top = p->top;
			vram_top += scroll_line;
			if(vram_top >= RES_Y) {
				vram_top -= RES_Y;
			}
			super_roll_put_tiny(p->left, vram_top, p->patnum);
			p->frames_left--;
			if(
				((p->left - 16) <= player_topleft.x) &&
				(p->left >= player_topleft.x) &&
				((p->top - 16) < player_topleft.y) &&
				(p->top > player_topleft.y)
			) {
				player_is_hit = true;
			}
			continue;
		}
		if(p->frame == 0) {
			snd_se_play(10);
			p->left -= 8;
			p->top -= 8;
		} else if(p->frame >= 32) {
			goto fire;
		}
		patnum = ((p->frame >> 2) + 26);
		vram_top = p->top;
		vram_top += scroll_line;
		if(vram_top >= RES_Y) {
			vram_top -= RES_Y;
		}
		super_roll_put(p->left, vram_top, patnum);
		p->frame++;
		continue;

fire:
		if((meira_flower_fire_i & 1) <= rank) {
			vram_top = p->top;
			vram_top += 12;
			bullets_add_pellet(
				(p->left + 12), vram_top, p->angle, p->group, p->speed
			);
		}
		meira_flower_fire_i++;
		p->flag = 2;
	}
}

void near meira_1483B(void)
{
	if(boss_phase_frame < 10) {
		return;
	}
	if(boss_phase_frame == 10) {
		snd_se_play(9);
		patnum_2064E = 142;
		return;
	}
	if(boss_phase_frame < 50) {
		return;
	}
	if(boss_phase_frame == 50) {
		snd_se_play(3);
		patnum_2064E = 143;
		return;
	}
	if(boss_phase_frame <= 90) {
		*boss_top_on_back_page += 6;
		return;
	}
	patnum_2064E = 141;
	boss_phase_frame = 0;
}

void pascal near meira_move(uint8_t dir, pixel_t delta_x, pixel_t delta_y)
{
	if(dir == 0) {
		*boss_left_on_back_page += delta_x;
		if(*boss_left_on_back_page > (PLAYFIELD_RIGHT - 48)) {
			*boss_left_on_back_page = (PLAYFIELD_RIGHT - 48);
		}
	} else {
		*boss_left_on_back_page -= delta_x;
		if(*boss_left_on_back_page < (PLAYFIELD_LEFT - 16)) {
			*boss_left_on_back_page = (PLAYFIELD_LEFT - 16);
		}
	}
	*boss_top_on_back_page += delta_y;
	meira_flower_add(
		(*boss_left_on_back_page + 12),
		(*boss_top_on_back_page + 12),
		0x00,
		meira_flower_group,
		meira_flower_speed,
		meira_flower_frames
	);
}

void near meira_148FD(void)
{
	register screen_y_t top;

	if(boss_phase_frame < 20) {
		return;
	}
	top = *boss_top_on_back_page;
	if(boss_phase_frame == 20) {
		meira_dir = ((*boss_left_on_back_page + 16) < player_topleft.x);
		patnum_2064E = (meira_dir + 144);
		meira_flower_frames = 16;
		meira_flower_patnum = (0x79 - meira_dir);
		meira_flower_group = 0x19;
		meira_flower_speed = 0x40;
	} else if(boss_phase_frame <= 32) {
		meira_move(meira_dir, -8, -8);
		meira_flower_frames += 2;
	} else if(boss_phase_frame < 56) {
		patnum_2064E = 141;
	} else if(boss_phase_frame == 56) {
		patnum_2064E = (145 - meira_dir);
		meira_flower_patnum = (meira_dir + 0x78);
		meira_flower_frames = 16;
	} else if(boss_phase_frame == 57) {
		meira_move(meira_dir, 8, -8);
		meira_flower_frames += 2;
		if(top > 48) {
			boss_phase_frame = 56;
		}
	} else if(boss_phase_frame < 80) {
		patnum_2064E = 141;
	} else if(boss_phase_frame == 80) {
		patnum_2064E = (149 - meira_dir);
		meira_flower_patnum = (0x79 - meira_dir);
		meira_flower_frames = 16;
	} else if(
		(*boss_left_on_back_page > 32) && (*boss_left_on_back_page < 352)
	) {
		meira_move(meira_dir, 8, 8);
		meira_flower_frames += 2;
	} else if(top > 48) {
		*boss_top_on_back_page -= 2;
		patnum_2064E = 141;
	} else {
		boss_phase_frame = 0;
	}
}

void near meira_14A39(void)
{
	if(boss_phase_frame < 50) {
		return;
	}
	if(boss_phase_frame == 50) {
		snd_se_play(9);
		patnum_2064E = 142;
		meira_flower_group = byte_2066E;
		return;
	}
	if(boss_phase_frame < 99) {
		return;
	}
	if(boss_phase_frame == 99) {
		snd_se_play(3);
		patnum_2064E = (149 - meira_dir);
		meira_flower_patnum = (meira_dir + 0x78);
		meira_flower_frames = 80;
		return;
	}
	if(boss_phase_frame < 120) {
		meira_move(meira_dir, -8, 8);
		meira_flower_frames--;
		return;
	}
	if(boss_phase_frame < 136) {
		patnum_2064E = 142;
		patnum_2064E = (meira_dir + 144);
		meira_flower_patnum = (0x79 - meira_dir);
		meira_flower_frames--;
		if(boss_phase_frame == 135) {
			snd_se_play(3);
		}
		return;
	}
	if(boss_phase_frame < 156) {
		meira_move(meira_dir, -8, -8);
		meira_flower_frames--;
		return;
	}
	if(*boss_left_on_back_page != 192) {
		patnum_2064E = 141;
		*boss_left_on_back_page += ((meira_dir == 0) ? 2 : -2);
		return;
	}
	boss_phase_frame = 0;
}

void near meira_14B33(void)
{
	if(boss_phase_frame < 50) {
		return;
	}
	if(boss_phase_frame == 50) {
		snd_se_play(9);
		patnum_2064E = 142;
		return;
	}
	if(boss_phase_frame < 99) {
		return;
	}
	if(boss_phase_frame == 99) {
		snd_se_play(3);
		patnum_2064E = 143;
		meira_speed_1 = (2 << 4);
		return;
	}
	if(boss_phase_frame < 116) {
		if(boss_phase_frame & 1) {
			bullets_add_pellet(
				(*boss_left_on_back_page + 24),
				(*boss_top_on_back_page + 32),
				0x40,
				byte_2066F,
				meira_speed_1
			);
			meira_speed_1 += 11;
		}
		return;
	}
	patnum_2064E = 141;
	boss_phase_frame = 0;
}

void near meira_14BC2(void)
{
	if(boss_phase_frame < 50) {
		return;
	}
	if(boss_phase_frame == 50) {
		snd_se_play(9);
		patnum_2064E = 142;
		meira_flower_group = 0x1A;
		return;
	}
	if(boss_phase_frame < 99) {
		return;
	}
	if(boss_phase_frame == 99) {
		snd_se_play(3);
		patnum_2064E = 143;
		meira_speed_2 = (2 << 4);
		return;
	}
	if(boss_phase_frame < 120) {
		if(boss_phase_frame & 1) {
			bullets_add_pellet(
				(*boss_left_on_back_page + 24),
				(*boss_top_on_back_page + 32),
				0x00,
				group_20670,
				meira_speed_2
			);
			meira_speed_2 += 11;
		}
		if(*boss_left_on_back_page != 192) {
			*boss_left_on_back_page += (
				(*boss_left_on_back_page < 192) ? 8 : -8
			);
		}
		return;
	}
	patnum_2064E = 141;
	boss_phase_frame = 0;
}

void near meira_14C76(void)
{
	if(boss_phase_frame < 50) {
		return;
	}
	if(boss_phase_frame == 50) {
		meira_dir = ((*boss_left_on_back_page + 16) < player_topleft.x);
		snd_se_play(9);
		patnum_2064E = 142;
		meira_flower_group = 0x1A;
		meira_flower_speed = 0x30;
		return;
	}
	if(boss_phase_frame < 99) {
		return;
	}
	if(boss_phase_frame == 99) {
		snd_se_play(3);
		patnum_2064E = (meira_dir + 148);
		meira_flower_patnum = (meira_dir + 0x78);
		meira_flower_frames = 20;
		return;
	}
	if(boss_phase_frame < 116) {
		meira_move(meira_dir, -8, 8);
		meira_flower_frames += 2;
		return;
	}
	if(boss_phase_frame < 136) {
		patnum_2064E = 142;
		patnum_2064E = (149 - meira_dir);
		meira_flower_patnum = (0x79 - meira_dir);
		meira_flower_frames = 20;
		if(boss_phase_frame == 135) {
			snd_se_play(3);
		}
		return;
	}
	if(boss_phase_frame < 152) {
		meira_move(meira_dir, 8, 8);
		meira_flower_frames += 2;
		return;
	}
	if(boss_phase_frame < 172) {
		patnum_2064E = 142;
		patnum_2064E = (145 - meira_dir);
		meira_flower_patnum = (meira_dir + 0x78);
		meira_flower_frames = 20;
		if(boss_phase_frame == 171) {
			snd_se_play(3);
		}
		return;
	}
	if(boss_phase_frame < 188) {
		meira_move(meira_dir, 8, -8);
		meira_flower_frames += 2;
		return;
	}
	if(boss_phase_frame < 208) {
		patnum_2064E = 142;
		patnum_2064E = (meira_dir + 144);
		meira_flower_patnum = (0x79 - meira_dir);
		meira_flower_frames = 20;
		if(boss_phase_frame == 207) {
			snd_se_play(3);
		}
		return;
	}
	if(boss_phase_frame < 224) {
		meira_move(meira_dir, -8, -8);
		meira_flower_frames += 2;
		return;
	}
	patnum_2064E = 141;
	boss_phase_frame = 0;
}

void near meira_14DFC(void)
{
	snd_se_play(3);
	patnum_2064E = 143;
	bullets_add_pellet(
		(*boss_left_on_back_page + 24),
		(*boss_top_on_back_page + 32),
		0x00,
		group_20670,
		((4 << 4) + 10)
	);
}

void near meira_14E30(void)
{
	if(boss_phase_frame < 50) {
		return;
	}
	if(boss_phase_frame == 50) {
		snd_se_play(9);
		patnum_2064E = 142;
		meira_flower_group = 0x1A;
		return;
	}
	if(boss_phase_frame < 99) {
		return;
	}
	if(boss_phase_frame == 99) {
		meira_14DFC();
		return;
	}
	if(boss_phase_frame < 108) {
		patnum_2064E = 142;
		return;
	}
	if(boss_phase_frame == 108) {
		meira_14DFC();
		return;
	}
	if(boss_phase_frame < 116) {
		patnum_2064E = 142;
		return;
	}
	if(boss_phase_frame == 116) {
		meira_14DFC();
		return;
	}
	patnum_2064E = 141;
	boss_phase_frame = 0;
}

void near meira_14E9D(void)
{
	register int i;

	if(boss_phase_frame < 50) {
		return;
	}
	if(boss_phase_frame == 50) {
		snd_se_play(9);
		patnum_2064E = 142;
		bullet_special.u3.turns_max = 5;
	} else if(boss_phase_frame >= 99) {
		patnum_2064E = 146;
		for(i = 0; i < 3; i++) {
			bullets_add_16x16(
				(*boss_left_on_back_page + 24),
				(*boss_top_on_back_page + 24),
				randring2_next8(),
				BSM_BOUNCE_LEFT_RIGHT_TOP_BOTTOM,
				((i & 1) + PAT_BULLET16_BILLIARD_BALL_RED),
				((3 << 4) + 6)
			);
		}
		boss_phase_frame = 0;
		meira_trail_active = true;
	}
}

void near meira_14F16(void)
{
	int delta_x;
	screen_y_t top;
	screen_x_t near *trail_left;
	screen_y_t near *trail_top;
	register int i;

	if(boss_phase_frame < 50) {
		return;
	}
	trail_left = meira_trail_left[page_back];
	trail_top = meira_trail_top[page_back];
	if((boss_phase_frame & 7) == 0) {
		trail_left[0] = trail_left[1];
		trail_top[0] = trail_top[1];
		trail_left[1] = trail_left[2];
		trail_top[1] = trail_top[2];
		trail_left[2] = *boss_left_on_back_page;
		trail_top[2] = *boss_top_on_back_page;
	}
	if(boss_phase_frame == 50) {
		for(i = 0; i < MEIRA_TRAIL_COUNT; i++) {
			trail_left[i] = *boss_left_on_back_page;
			trail_top[i] = *boss_top_on_back_page;
		}
		meira_target_left = ((randring2_next16() % 320) + 32);
		meira_target_top = (randring2_next8() + 16);
		meira_origin_left = *boss_left_on_back_page;
		meira_origin_top = *boss_top_on_back_page;
		meira_step = 1;
	} else if(boss_phase_frame <= 114) {
		delta_x = (((meira_target_left - meira_origin_left) * meira_step) / 64);
		top = (((meira_target_top - meira_origin_top) * meira_step) / 64);
		top += meira_origin_top;
		*boss_left_on_back_page = (delta_x + meira_origin_left);
		*boss_top_on_back_page = top;
		meira_step++;
	} else if(boss_phase_frame == 114) {
		snd_se_play(9);
	} else if(boss_phase_frame < 164) {
		return;
	} else {
		patnum_2064E = 146;
		for(i = 0; i < 2; i++) {
			bullets_add_16x16(
				(*boss_left_on_back_page + 24),
				(*boss_top_on_back_page + 24),
				randring2_next8(),
				BSM_BOUNCE_LEFT_RIGHT_TOP_BOTTOM,
				(i + PAT_BULLET16_BILLIARD_BALL_RED),
				((3 << 4) + 6)
			);
		}
		boss_phase_frame = 0;
	}
	for(i = 0; i < MEIRA_TRAIL_COUNT; i++) {
		if(
			(trail_left[i] != *boss_left_on_back_page) &&
			(trail_top[i] != *boss_top_on_back_page)
		) {
			top = trail_top[i];
			top += scroll_line;
			if(top >= RES_Y) {
				top -= RES_Y;
			}
			super_roll_put_1plane(trail_left[i], top, 147, 0, super_plane(14));
		}
	}
}

int meira_update(void)
{
	register screen_y_t target_top;

	boss_phase_frame++;
	target_top = *boss_top_on_back_page;
	target_top += 40;
	shot_target_left = (*boss_left_on_back_page + 40);
	shot_target_top = target_top;
	if(byte_2066A) {
		if(meira_defeat_animate()) {
			return 2;
		}
		goto flowers;
	}
	if(meira_phase == 0) {
		switch(meira_pattern) {
		case 0:	meira_1483B();	break;
		case 1:	meira_148FD();	break;
		case 2:	meira_14A39();	break;
		case 3:	meira_14B33();	break;
		}
		if(boss_phase_frame == 0) {
			meira_pattern++;
			if(meira_pattern > 3) {
				meira_pattern = 0;
			}
			if((boss_damage > 700) && (meira_pattern != 1)) {
				meira_phase++;
				meira_pattern = 0;
			}
		}
	} else if(meira_phase == 1) {
		switch(meira_pattern) {
		case 0:	meira_14BC2();	break;
		case 1:	meira_14C76();	break;
		case 2:	meira_14E30();	break;
		}
		if(boss_phase_frame == 0) {
			meira_pattern++;
			if(meira_pattern > 2) {
				meira_pattern = 0;
			}
			if(boss_damage > 1500) {
				meira_phase++;
				meira_pattern = 0;
			}
		}
	} else if(meira_phase == 2) {
		switch(meira_pattern) {
		case 0:	meira_14E9D();	break;
		case 1:	meira_14F16();	break;
		}
		if(boss_phase_frame == 0) {
			meira_pattern = 1;
		}
	}
	meira_hittest_and_render();

flowers:
	meira_flowers_update_and_render();
	return 1;
}

void meira_end(void)
{
	dialog_pre();
	dialog_script_generic_part_animate(DS_POSTBOSS);
	stage_clear_bonus_animate();
	key_delay();
	overlay_stage_leave_animate();
	stage_id++;
	spark_accel_x.v = 0;
}

void meira_init(void)
{
	register vram_y_t vram_top;

	patnum_2064E = 141;
	dialog_pre();
	dialog_script_stage2_pre_intro_animate();
	boss_left_on_page[0] = (PLAYFIELD_LEFT + (PLAYFIELD_W / 2) - 32);
	boss_left_on_page[1] = boss_left_on_page[0];
	boss_top_on_page[0] = (PLAYFIELD_TOP + 32);
	boss_top_on_page[1] = (PLAYFIELD_TOP + 32);
	boss_left_on_back_page = &boss_left_on_page[page_back];
	boss_top_on_back_page = &boss_top_on_page[page_back];
	palette_white_out(1);
	vram_top = 48;
	vram_top += scroll_line;
	if(vram_top >= RES_Y) {
		vram_top -= RES_Y;
	}
	super_roll_put(*boss_left_on_back_page, vram_top, patnum_2064E);
	sub_13ABB("boss4.m");
	palette_white_in(1);
	dialog_script_generic_part_animate(DS_PREBOSS);
	dialog_post();
	boss_damage = 0;
	byte_2066A = false;
	bullet_special.u3.turns_max = 2;
	angle_1E510 = 0;
	meira_phase = 0;
	meira_pattern = 0;
	meira_trail_active = false;
	if(rank != RANK_EASY) {
		byte_2066E = 0x1A;
		byte_2066F = 0x15;
		group_20670 = BG_5_SPREAD_MEDIUM_AIMED;
	} else {
		byte_2066E = 0x1A;
		byte_2066F = 0x09;
		group_20670 = BG_3_SPREAD_MEDIUM_AIMED;
	}
}
