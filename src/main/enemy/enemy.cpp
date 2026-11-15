/* ReC98
 * -----
 * Stage enemy updates, collision, rendering and invalidation of TH02's
 * MAIN.EXE
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
#include "th02/hardware/pages.hpp"
#include "th02/math/randring.hpp"
#include "th02/math/vector.hpp"
#include "th02/snd/snd.h"
#include "th02/main/playfld.hpp"
#include "th02/main/score.hpp"
#include "th02/main/scroll.hpp"
#include "th02/main/spark.hpp"
#include "th02/main/bullet/bullet.hpp"
#include "th02/main/enemy/enemy.hpp"
#include "th02/main/item/item.hpp"
#include "th02/main/player/player.hpp"
#include "th02/main/tile/tile.hpp"
#include "th02/main/midboss/midboss.hpp"

extern screen_x_t shot_target_left;
extern screen_y_t shot_target_top;

int pascal near shots_hittest(
	screen_x_t left, screen_y_t top, pixel_t w, pixel_t h
);
void pascal items_add(screen_x_t left, screen_y_t top, int type);

static const int STAGE_LINE_COLUMNS = 49;
extern int stage_line_count;
// Uninitialized data, in the original order
enemy_t enemies[ENEMY_COUNT];
stage_formation_t stage_formations[STAGE_FORMATION_COUNT];
int16_t far *stage_lines[STAGE_LINE_COLUMNS];
int stage_line_count;
void (far *farfp_26C3C)(void);
void (far *farfp_26C40)(void);
unsigned char near *enemy_script;
enemy_t near *enemy_cur;
stage_formation_t near *enemy_formation;
screen_x_t near *enemy_left_on_back_page;
screen_y_t near *enemy_top_on_back_page;
screen_x_t enemy_left;
screen_y_t enemy_top;
int8_t near *enemy_velocity_x;
int8_t near *enemy_velocity_y;

int stage_line_cur = 0;
uint8_t enemy_count = 0;
extern uint8_t scroll_speed_this_frame;
extern int tile_rows_scrolled;
extern void (far *farfp_26C3C)(void);
extern void (far *farfp_26C40)(void);

// Script of the enemy currently being updated.

void enemies_nop(void)
{
}

void sub_16A6B(void)
{
	int i;
	for(i = 0; i < ENEMY_COUNT; i++) {
		enemies[i].flag = EF_FREE;
	}
	stage_line_cur = 0;
}

void sub_16A8A(void)
{
	farfp_26C3C = enemies_nop;
	farfp_26C40 = enemies_nop;
}

void pascal near enemy_aimed_pellet_add(unsigned char angle)
{
	bullets_add_pellet(
		(enemy_formation->bullet_offset.x + *enemy_left_on_back_page),
		(enemy_formation->bullet_offset.y + *enemy_top_on_back_page),
		angle,
		enemy_cur->bullet_group,
		enemy_cur->bullet_speed
	);
}

void near enemy_move(void)
{
	*enemy_left_on_back_page += *enemy_velocity_x;
	*enemy_top_on_back_page += *enemy_velocity_y;
}

void near enemy_move_aimed(void)
{
	screen_y_t player_center_y;
	int vector_x;
	int vector_y;

	player_center_y = (player_topleft.y + 16);
	vector2_between_plus(
		*enemy_left_on_back_page,
		*enemy_top_on_back_page,
		(player_topleft.x + 24),
		player_center_y,
		0x00,
		vector_x,
		vector_y,
		enemy_script[enemy_cur->script_ip + 2]
	);
	*enemy_velocity_x = vector_x;
	*enemy_velocity_y = vector_y;
	*enemy_left_on_back_page += vector_x;
	*enemy_top_on_back_page += vector_y;
}

void pascal near enemy_move_polar(int param_offset, int angle_delta)
{
	int vector_x;
	int vector_y;
	unsigned char angle;
	register int p;

	p = (enemy_cur->script_ip + param_offset);
	if((enemy_cur->kill_frame % enemy_script[p + 1]) == 0) {
		enemy_cur->loop_i += angle_delta;
	}
	angle = (enemy_script[p] + enemy_cur->loop_i);
	vector2(vector_x, vector_y, angle, enemy_script[p + 2]);
	*enemy_left_on_back_page += vector_x;
	*enemy_top_on_back_page += vector_y;
	*enemy_velocity_x = vector_x;
	*enemy_velocity_y = vector_y;
}

void near enemy_move_polar_swing(void)
{
	screen_x_t far *left;
	screen_y_t far *top;
	int vector_x;
	int vector_y;
	int sign;
	unsigned char angle;
	register int p;

	left = enemy_left_on_back_page;
	top = enemy_top_on_back_page;
	p = (enemy_cur->script_ip + 2);
	sign = ((enemy_script[p++] == 0) ? -1 : 1);
	angle = (enemy_script[p] + enemy_cur->loop_i);
	p++;
	enemy_cur->loop_i += (enemy_script[p] * sign);
	p++;
	vector2(vector_x, vector_y, angle, enemy_script[p]);
	*left += vector_x;
	*top += vector_y;
	*enemy_velocity_x = vector_x;
	*enemy_velocity_y = vector_y;
}

void pascal near enemy_add(
	screen_x_t left, screen_y_t top, int slot, int formation
)
{
	enemies[slot].flag = EF_ALIVE;
	enemies[slot].pos[page_back].x = left;
	enemies[slot].pos[page_back].y = top;
	enemies[slot].cel = 0;
	enemies[slot].script_ip = 0;
	enemies[slot].formation = formation;
	enemies[slot].kill_frame = 0;
	enemies[slot].age = randring2_next8();
	enemies[slot].loop_i = 0;
	enemies[slot].hittest_disabled = false;
	enemies[slot].player_collision_disabled = false;
	enemies[slot].spawned_left = ((left < 208) ? 1 : 0);
	enemies[slot].loop_j = 0;
	enemies[slot].clip_y = false;
	enemies[slot].anim_mode = 1;
	enemies[slot].dying = false;
	enemies[slot].damage = 0;

	// ZUN bug: Should be `<=`.
	if(enemy_count < slot) {
		enemy_count = (slot + 1);
	}
}

void sub_16D9B(void)
{
	int formation;
	screen_y_t top;
	register int slot;
	register int column;

	if(stage_line_cur < stage_line_count) {
		if(stage_lines[0][stage_line_cur] == tile_rows_scrolled) {
			for(column = 1; column < STAGE_LINE_COLUMNS; column++) {
				if(stage_lines[column][stage_line_cur] != -1) {
					formation = stage_lines[column][stage_line_cur];
					top = stage_formations[formation].unknown_00[1];
					if(midboss_active) {
						continue;
					}
					for(slot = 0; slot < ENEMY_COUNT; slot++) {
						if(enemies[slot].flag == EF_FREE) {
							enemy_add(((column * 8) + 16), top, slot, formation);
							break;
						}
					}
				}
			}
			stage_line_cur++;
		}
	}
}

int near enemy_script_run(void)
{
	int sign;
	unsigned char angle;
	uint8_t len;
	uint8_t speed;
	int8_t velocity_y;
	uint8_t ip;
	uint8_t cmd;
	int8_t sign_polar_1;
	int8_t sign_polar_2;
	register screen_y_t top;

	#define param(offset) enemy_script[ip + offset]

	enemy_script = reinterpret_cast<unsigned char near *>(
		enemy_formation->script
	);
	ip = enemy_cur->script_ip;
	cmd = enemy_script[ip];
	switch(cmd) {
	case 0x00:
		len = 2;
		break;
	case 0x01:
		*enemy_velocity_x = param(2);
		velocity_y = param(3);
		*enemy_velocity_y = velocity_y;
		enemy_move();
		len = 4;
		break;
	case 0x0A:
		enemy_move();
		len = 2;
		break;
	case 0xAA:
		angle = randring2_next8();
		speed = param(1);
		*enemy_velocity_x = ((speed * CosTable8[angle]) >> 8);
		*enemy_velocity_y = ((speed * SinTable8[angle]) >> 8);
		len = 2;
		goto advance;
	case 0x08:
		sign = ((enemy_cur->spawned_left == 0) ? -1 : 1);
		*enemy_velocity_x = (param(2) * sign);
		*enemy_velocity_y = param(3);
		enemy_move();
		len = 4;
		break;
	case 0x09:
		enemy_move_aimed();
		len = 3;
		break;
	case 0x02:
		*enemy_top_on_back_page += scroll_speed_this_frame;
		len = 2;
		break;
	case 0x03:
		sign_polar_1 = ((enemy_script[enemy_cur->script_ip + 2] == 0) ? -1 : 1);
		enemy_move_polar(3, sign_polar_1);
		len = 6;
		break;
	case 0x07:
		sign_polar_2 = ((enemy_cur->spawned_left == 1) ? -1 : 1);
		enemy_move_polar(2, sign_polar_2);
		len = 5;
		break;
	case 0x0B:
		enemy_move_polar_swing();
		len = 6;
		break;
	case 0x04:
		*enemy_left_on_back_page = (param(1) * 8);
		*enemy_top_on_back_page = (param(2) * 8);
		len = 3;
		goto advance;
	case 0x05:
		*enemy_left_on_back_page = (
			(randring2_next16() % (param(2) * 8)) + (param(1) * 8)
		);
		*enemy_top_on_back_page = (param(3) * 8);
		enemy_cur->spawned_left = ((*enemy_left_on_back_page < 208) ? 1 : 0);
		len = 4;
		goto advance;
	case 0x06:
		*enemy_top_on_back_page = (
			(randring2_next16() % (param(2) * 8)) + (param(1) * 8)
		);
		*enemy_left_on_back_page = (param(3) * 8);
		len = 4;
		goto advance;
	case 0x10:
		enemy_cur->flag = EF_REMOVE;
		return 2;
	case 0x11:
		enemy_cur->kill_frame = 0;
		snd_se_play(3);
		enemy_cur->dying = true;
		return 2;
	case 0x20:
		enemy_aimed_pellet_add(0x00);
		goto advance_1;
	case 0x21:
		enemy_aimed_pellet_add(param(1));
		len = 2;
		goto advance;
	case 0x22:
		enemy_aimed_pellet_add(enemy_cur->loop_i);
		enemy_cur->loop_i += param(1);
		len = 2;
		goto advance;
	case 0x25:
		enemy_cur->bullet_group = param(1);
		enemy_cur->bullet_speed = param(2);
		len = 3;
		goto advance;
	case 0x24:
		bullets_add_16x16(
			(enemy_formation->bullet_offset.x + *enemy_left_on_back_page),
			(enemy_formation->bullet_offset.y + *enemy_top_on_back_page),
			param(1),
			static_cast<bullet_group_or_special_motion_t>(
				enemy_cur->bullet_group
			),
			param(2),
			enemy_cur->bullet_speed
		);
		len = 3;
		goto advance;
	case 0x26:
		top = param(1);
		top -= scroll_line;
		if(top < 0) {
			top += RES_Y;
		}
		bullets_add_pellet(
			(enemy_formation->bullet_offset.x + *enemy_left_on_back_page),
			top,
			param(2),
			param(3),
			param(4)
		);
		len = 5;
		goto advance;
	case 0x27:
		top = param(1);
		top -= scroll_line;
		if(top < 0) {
			top += RES_Y;
		}
		bullets_add_16x16(
			(enemy_formation->bullet_offset.x + *enemy_left_on_back_page),
			top,
			param(2),
			static_cast<bullet_group_or_special_motion_t>(param(3)),
			param(4),
			param(5)
		);
		len = 6;
		goto advance;
	case 0xA0:
		enemy_cur->script_ip = param(1);
		return 1;
	case 0xA1:
		if(enemy_cur->loop_i++ < param(2)) {
			enemy_cur->script_ip = param(1);
		} else {
			enemy_cur->script_ip += 3;
		}
		return 1;
	case 0xA7:
		if(enemy_cur->loop_j++ < param(2)) {
			enemy_cur->script_ip = param(1);
		} else {
			enemy_cur->loop_j = 0;
			enemy_cur->script_ip += 3;
		}
		return 1;
	case 0xA2:
		enemy_cur->loop_i = param(1);
		len = 2;
		goto advance;
	case 0xA3:
		enemy_cur->anim_mode = 0;
		goto advance_1;
	case 0xA4:
		enemy_cur->anim_mode = 1;
		goto advance_1;
	case 0xA5:
		enemy_cur->cel = param(1);
		enemy_cur->anim_mode = 0;
		len = 2;
		goto advance;
	case 0xA6:
		enemy_cur->cel = param(1);
		enemy_cur->anim_mode = 2;
		len = 2;
		goto advance;
	case 0xA9:
		enemy_cur->clip_y = true;
		goto advance_1;
	case 0xA8:
		snd_se_play(param(1));
		len = 2;
		goto advance;
	case 0xAB:
		enemy_cur->hittest_disabled = true;
		goto advance_1;
	case 0xAC:
		enemy_cur->hittest_disabled = false;
		goto advance_1;
	case 0xAD:
		enemy_cur->player_collision_disabled = true;
		goto advance_1;
	case 0xAE:
		enemy_cur->player_collision_disabled = false;
		goto advance_1;
	case 0xAF:
		tile_ring_set_and_put_both_8(*enemy_left_on_back_page, param(1), param(2));
		len = 3;
		goto advance;
	}

	// Waiting command
	enemy_cur->kill_frame++;
	if(param(1) <= enemy_cur->kill_frame) {
		enemy_cur->script_ip += len;
		enemy_cur->kill_frame = 0;
	}
	return 0;

advance:
	enemy_cur->script_ip += len;
	return 1;

advance_1:
	enemy_cur->script_ip++;
	return 1;

	#undef param
}

int near enemy_hittest(void)
{
	int damage;
	register screen_y_t top;

	if(
		!enemy_formation->player_collision_disabled &&
		!enemy_cur->player_collision_disabled
	) {
		top = enemy_top;
		if(
			(player_left_on_page[page_front] > (enemy_left - 16)) &&
			(player_left_on_page[page_front] < (enemy_left + 16)) &&
			((top - 16) < player_topleft.y) &&
			((top + 16) > player_topleft.y)
		) {
			enemy_cur->kill_frame = 0;
			player_is_hit = true;
			enemy_cur->dying = true;
		}
	}
	damage = shots_hittest(enemy_left, enemy_top, 32, 32);
	return damage;
}

int near enemy_kill_animate(void)
{
	register int patnum;
	register vram_y_t vram_top;

	patnum = enemy_formation->kill_patnum_offset_unused;
	patnum = ((patnum == 0) ? 10 : 0);
	patnum += (enemy_cur->kill_frame / 3);
	if(enemy_cur->kill_frame == 0) {
		sparks_add(
			(enemy_left + 16), (enemy_top + 16), ((1 << 4) + 4), 8, false
		);
	}
	enemy_cur->kill_frame++;
	if(enemy_cur->kill_frame >= 24) {
		enemy_cur->flag = EF_REMOVE;
		return 1;
	}
	vram_top = enemy_top;
	vram_top += scroll_line;
	if(vram_top >= RES_Y) {
		vram_top -= RES_Y;
	}
	super_roll_put(enemy_left, vram_top, patnum);
	return 0;
}

void sub_1766E(void)
{
	int script_ret = 0;
	screen_y_t target_top_max = 0;
	int damage;
	int interval;
	register screen_y_t top;
	register int i;

	shot_target_left = -1;
	shot_target_top = -1;
	enemy_cur = enemies;
	for(i = 0; i < enemy_count; (i++, enemy_cur++)) {
		if(enemy_cur->flag != EF_ALIVE) {
			continue;
		}
		enemy_formation = &stage_formations[enemy_cur->formation];
		enemy_left_on_back_page = &enemy_cur->pos[page_back].x;
		enemy_top_on_back_page = &enemy_cur->pos[page_back].y;
		enemy_velocity_x = &enemy_cur->velocity_x;
		enemy_velocity_y = &enemy_cur->velocity_y;
		do {
			if(!enemy_cur->dying) {
				script_ret = enemy_script_run();
			}
			if(script_ret == 2) {
				goto next;
			}
		} while(script_ret == 1);
		enemy_left = *enemy_left_on_back_page;
		enemy_top = *enemy_top_on_back_page;
		top = enemy_top;
		if(
			(enemy_left < (PLAYFIELD_LEFT - 32)) ||
			(enemy_left > (PLAYFIELD_RIGHT + 2))
		) {
			enemy_cur->flag = EF_REMOVE;
			continue;
		}
		if(
			enemy_cur->clip_y && (
				(enemy_top >= PLAYFIELD_BOTTOM) ||
				(enemy_top <= (PLAYFIELD_TOP - 32))
			)
		) {
			enemy_cur->flag = EF_REMOVE;
			continue;
		}
		enemy_cur->age++;
		if(
			(enemy_formation->cels != 0) &&
			(enemy_cur->anim_mode == 1) &&
			((enemy_cur->age % enemy_formation->frames_per_cel) == 0)
		) {
			enemy_cur->cel++;
			if(enemy_cur->cel >= enemy_formation->cels) {
				enemy_cur->cel = 0;
			}
		}
		if((enemy_formation->hp == -1) || enemy_cur->hittest_disabled) {
			goto render;
		}
		if(enemy_cur->dying) {
			// ZUN bloat: Both outcomes continue with the next enemy.
			if(enemy_kill_animate()) {
				continue;
			}
			continue;
		}
		if((top < 360) && (top > target_top_max)) {
			shot_target_left = (enemy_left + 8);
			shot_target_top = (top + 8);
			target_top_max = top;
		}
		if((damage = enemy_hittest()) != 0) {
			if(enemy_formation->hp == -2) {
				snd_se_play(4);
				goto render;
			}
			enemy_cur->damage += damage;
			if(enemy_cur->damage >= enemy_formation->hp) {
				enemy_cur->kill_frame = 0;
				if(enemy_formation->item > 1) {
					items_add(
						(enemy_left + 12),
						enemy_top,
						(enemy_formation->item - IT_BOMB)
					);
				} else {
					items_add_semirandom((enemy_left + 12), enemy_top);
				}
				snd_se_play(3);
				score_delta += enemy_formation->score;
				if(rank == RANK_LUNATIC) {
					bullets_add_pellet(
						(enemy_formation->bullet_offset.x + enemy_left),
						(enemy_formation->bullet_offset.y + enemy_top),
						0x00,
						BG_1_AIMED,
						((3 << 4) + 12)
					);
				}
				enemy_cur->dying = true;
			} else {
				snd_se_play(4);
				top += scroll_line;
				if(top >= RES_Y) {
					top -= RES_Y;
				}
				if(top < 0) {
					top += RES_Y;
				}
				super_roll_put_1plane(
					enemy_left,
					top,
					(enemy_formation->patnum + enemy_cur->cel),
					0,
					super_plane(V_WHITE)
				);
			}
			continue;
		}
render:
		top = enemy_top;
		if(enemy_formation->speed > 1) {
			interval = enemy_formation->speed;
			if((enemy_cur->age % interval) == 0) {
				enemy_aimed_pellet_add(0x00);
			}
		}
		if(enemy_formation->patnum != 0) {
			top += scroll_line;
			if(top >= RES_Y) {
				top -= RES_Y;
			}
			if(top < 0) {
				top += RES_Y;
			}
			if(enemy_cur->anim_mode != 2) {
				super_roll_put(
					enemy_left, top, (enemy_formation->patnum + enemy_cur->cel)
				);
			} else {
				super_roll_put(enemy_left, top, enemy_cur->cel);
			}
		}
next:
		;
	}
}

void sub_17979(void)
{
	int i;
	register screen_x_t left;
	register screen_y_t top;

	enemy_count = 0;
	for(i = 0; i < ENEMY_COUNT; i++) {
		if(enemies[i].flag == EF_FREE) {
			continue;
		}
		left = enemies[i].pos[page_back].x;
		top = enemies[i].pos[page_back].y;
		tiles_invalidate_rect(left, top, 32, 32);
		if(enemies[i].flag == EF_REMOVE) {
			enemies[i].flag = EF_FREE;
			continue;
		}
		enemy_count = (i + 1);
		enemies[i].pos[page_back].x = enemies[i].pos[page_front].x;
		enemies[i].pos[page_back].y = enemies[i].pos[page_front].y;
	}
}

void sub_17A55(void)
{
	int i;
	for(i = 0; i < ENEMY_COUNT; i++) {
		if(enemies[i].flag != EF_FREE) {
			enemies[i].flag = EF_REMOVE;
		}
	}
	enemy_count = 0;
}
