/// Stage 5 Boss - Mima
/// -------------------

#include "platform.h"
#include "pc98.h"
#include "libs/master.lib/master.hpp"
#include "libs/master.lib/pc98_gfx.hpp"
#include "th02/resident.hpp"
#include "th02/math/randring.hpp"
#include "th02/main/frames.hpp"
#include "th02/main/boss/boss.hpp"
#include "th02/main/player/player.hpp"
#include "th02/main/bullet/bullet.hpp"
#include "th02/hardware/pages.hpp"
#include "th02/core/globals.hpp"
#include "th02/snd/snd.h"
#include "th02/sprites/main_pat.h"
#include "th01/math/polar.hpp"
#include "th01/math/subpixel.hpp"
#include "th01/rank.h"
#include "th02/math/vector.hpp"
#include "th02/main/playfld.hpp"
#include "th02/main/item/item.hpp"
#include "th02/main/tile/tile.hpp"
#include "planar.h"

#include "th02/main/circle.hpp"

extern int16_t shot_target_left;
extern int16_t shot_target_top;
extern int16_t patnum_2064E;
extern int16_t word_1EE56;
extern uint8_t byte_20672;

extern uint8_t byte_20671;

extern uint8_t byte_2066E;
extern uint8_t byte_2066F[];
uint8_t angle_1EE54 = 0;
int16_t word_1EE56 = 0;
extern bool byte_2066A;
extern bool byte_2066B;


// Returns the damage dealt by the player's shots to the given rectangle.
int pascal near shots_hittest(
	screen_x_t left, screen_y_t top, pixel_t w, pixel_t h
);

static const int SPHERE_COUNT = 4;
static const int SPHERE_SLOTS = 8;

// Uninitialized data, in the original order
// -----------------------------------------

int16_t left_26C56;
int16_t word_26C58;
int16_t left_26C5A;
int16_t x_26C5C;
int16_t top_26C5E;
int16_t word_26C60;
int16_t top_26C62;
int16_t y_26C64;
int16_t word_26C66;
int16_t word_26C68;
int16_t word_26C6A;
screen_x_t mima_sphere_left[PAGE_COUNT][SPHERE_SLOTS];
screen_y_t mima_sphere_top[PAGE_COUNT][SPHERE_SLOTS];
int16_t mima_sphere_active[SPHERE_SLOTS];
int16_t word_26CBC;
int16_t word_26CBE;
uint8_t byte_26CC0;
uint8_t byte_26CC1;
uint8_t byte_26CC2;
uint8_t byte_26CC3;
uint8_t byte_26CC4;
uint8_t byte_26CC5;
uint8_t byte_26CC6;
uint8_t byte_26CC7;
uint8_t byte_26CC8;
uint8_t byte_26CC9;
uint8_t byte_26CCA;
int16_t word_26CCC;
int16_t word_26CCE;
int16_t word_26CD0;
uint8_t angle_26CD2;
int8_t byte_26CD3;
uint8_t byte_26CD4;
point_t point_26CD6;
uint8_t angle_26CDA;
uint8_t speed_26CDB;
uint8_t angle_26CDC;
point_t point_26CDE;
uint8_t angle_26CE2;
int8_t direction_26CE3;
uint8_t angle_26CE4;
int16_t word_26CE6;
int16_t word_26CE8;
int16_t word_26CEA;
uint8_t angle_26CEC;
uint8_t angle_26CED;
uint8_t angle_26CEE;
uint8_t byte_26CEF;
int16_t word_26CF0;
int16_t word_26CF2;
int16_t word_26CF4;
int16_t word_26CF6;
uint8_t byte_26CF8;
uint8_t byte_26CF9;
uint8_t byte_26CFA;
// -----------------------------------------

int near mima_19C8D(void);
void mima_end(void);

void pascal near mima_17A7F(
	int col_,
	unsigned char angle_offset,
	int angle_step,
	bool disc,
	unsigned int angle_start_
)
{
	register int col = col_;
	register unsigned int angle_start = angle_start_;
	unsigned int i;
	unsigned char angle;
	unsigned char size = ((byte_26CC1 >> 6) + 1);

	grcg_setcolor(GC_RMW, col);
	for(i = angle_start; (angle_start + 0x80) > i; i += angle_step) {
		angle = i;
		circle_dot_left = polar_x_fast(224, byte_26CC1, angle);
		angle += angle_offset;
		circle_dot_top = polar_y_fast(200, byte_26CC1, angle);
		if(
			(circle_dot_left < 0x1A0) && (circle_dot_left > 0x18) &&
			(circle_dot_top > 8) && (circle_dot_top < 0x180)
		) {
			circle_dot_put(size);
		}
		if(disc && (i == 0x100)) {
			grcg_setcolor(GC_RMW, byte_26CC5);
			grcg_circlefill(224, 200, byte_26CC2);
			grcg_setcolor(GC_RMW, col);
		}
	}
	grcg_off();
}

void near mima_17E91(void);

void mima_bg_render(void)
{
	unsigned char angle_step;

	mima_17E91();
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
	angle_step = ((reduce_effects * 3) + 1);
	mima_17A7F(byte_26CC4, (angle_1EE54 - 16), angle_step, false, 0x80);
	mima_17A7F(byte_26CC4, (angle_1EE54 - 8), angle_step, false, 0x80);
	mima_17A7F(byte_26CC3, angle_1EE54, angle_step, false, 0x80);
	mima_17A7F(byte_26CC4, (angle_1EE54 - 16), angle_step, true, 0x100);
	mima_17A7F(byte_26CC4, (angle_1EE54 - 8), angle_step, false, 0x100);
	mima_17A7F(byte_26CC3, angle_1EE54++, angle_step, false, 0x100);
	grcg_off();
}

void near mima_17C92(void)
{
	int damage;

	if(byte_2066A) {
		return;
	}
	if((damage = shots_hittest(
		(*boss_left_on_back_page + 48), (*boss_top_on_back_page + 32), 64, 64
	)) != 0) {
		byte_2066B = true;
		boss_damage += (word_26C66 * damage);
		if(boss_damage >= 6000) {
			snd_se_play(2);
			byte_2066A = true;
			score += 500000;
			player_invincibility_time = BOSS_DEFEAT_INVINCIBILITY_FRAMES;
		}
	}
	if(
		(player_left_on_page[page_front] > (*boss_left_on_back_page + 16)) &&
		(player_left_on_page[page_front] < (*boss_left_on_back_page + 112)) &&
		(player_top_on_page[page_front] > *boss_top_on_back_page) &&
		(player_top_on_page[page_front] < (*boss_top_on_back_page + 80))
	) {
		player_is_hit = true;
	}
}

void near mima_17D59(void)
{
	int i;

	word_26CBC = 0;
	for(i = 0; i < SPHERE_SLOTS; i++) {
		if(mima_sphere_active[i] != 1) {
			word_26CBC++;
			continue;
		}
		shots_hittest(
			(mima_sphere_left[page_back][i] + 4),
			mima_sphere_top[page_back][i],
			24,
			32
		);
		if(
			((mima_sphere_left[page_back][i] - 16) < player_left_on_page[page_front]) &&
			((mima_sphere_left[page_back][i] + 16) > player_left_on_page[page_front]) &&
			((mima_sphere_top[page_back][i] - 16) < player_top_on_page[page_front]) &&
			((mima_sphere_top[page_back][i] + 16) > player_top_on_page[page_front])
		) {
			player_is_hit = true;
		}
		super_put_rect(
			mima_sphere_left[page_back][i],
			mima_sphere_top[page_back][i],
			((i & 3) + 0x89)
		);
	}
}

void near mima_17E91(void)
{
	int i;

	for(i = 0; i < SPHERE_SLOTS; i++) {
		if(mima_sphere_active[i] == 0) {
			continue;
		}
		if(
			(mima_sphere_top[page_back][i] > -16) &&
			(mima_sphere_top[page_back][i] < 0x17E)
		) {
			tiles_invalidate_rect(
				mima_sphere_left[page_back][i], mima_sphere_top[page_back][i], 32, 32
			);
			if(mima_sphere_active[i] == 2) {
				mima_sphere_active[i] = 0;
			}
		}
	}
}

void near mima_17F27(void)
{
	int row;
	screen_y_t y;
	vram_offset_t vo;
	vram_offset_t vo_topleft;
	int part;
	uint8_t far *sprite;
	unsigned char shift;
	dots16_t dots;
	int col;
	screen_x_t x;

	if(byte_2066B) {
		byte_26CC6++;
		if((byte_26CC6 & 3) == 0) {
			shift = (*boss_left_on_back_page & 7);
			byte_2066B = false;
			vo_topleft = (
				(*boss_left_on_back_page >> 3) +
				(*boss_top_on_back_page << 6) +
				(*boss_top_on_back_page << 4)
			);
			grcg_setcolor(GC_RMW, 4);
			for(part = 0; part < 3; part++) {
				vo = ((part * 6) + vo_topleft);
				sprite = reinterpret_cast<uint8_t far *>(
					MK_FP(super_patdata[patnum_2064E + part], 0)
				);
				for(row = 0, y = *boss_top_on_back_page; row < 128; row++, vo += ROW_SIZE, y++) {
					if(y >= 0x180) {
						break;
					}
					for(
						col = 0, x = ((part * 48) + *boss_left_on_back_page);
						col < 6;
						col++, x += 8, sprite++
					) {
						if((x > 0) && (x < 0x1A0) && (y >= 16)) {
							dots = ((*sprite << (16 - shift)) + (*sprite >> shift));
							*reinterpret_cast<dots16_t far *>(
								(VRAM_PLANE_B + vo) + col
							) = dots;
						}
					}
				}
			}
			grcg_off();
			return;
		}
	}
	super_put_rect(
		boss_left_on_page[page_back], boss_top_on_page[page_back], patnum_2064E
	);
	super_put_rect(
		(boss_left_on_page[page_back] + 48),
		boss_top_on_page[page_back],
		(patnum_2064E + 1)
	);
	super_put_rect(
		(boss_left_on_page[page_back] + 96),
		boss_top_on_page[page_back],
		(patnum_2064E + 2)
	);
}

void near mima_180AC(void)
{
	if(rank != RANK_EASY) {
		byte_2066E = BG_16_RING;
		byte_2066F[0] = BG_5_SPREAD_MEDIUM_AIMED;
		byte_2066F[1] = BG_32_RING;
		byte_20671 = BG_4_RING;
		byte_20672 = 6;
	} else {
		byte_2066E = BG_16_RING;
		byte_2066F[0] = BG_1_AIMED;
		byte_2066F[1] = BG_2_SPREAD_MEDIUM_AIMED;
		byte_20671 = BG_2_RING;
		byte_20672 = 8;
	}
}

#define mima_move_toward_player() \
	*boss_left_on_back_page += ( \
		((*boss_left_on_back_page + 64) < player_topleft.x) ? 2 : -2 \
	);

void near mima_180EC(void)
{
	if(boss_phase_frame < 10) {
		return;
	}
	if(boss_phase_frame == 10) {
		bullets_set_stack_multiplier(0);
		patnum_2064E = 131;
	}
	if(boss_phase_frame < 66) {
		*boss_left_on_back_page -= 2;
		if(*boss_left_on_back_page <= 32) {
			boss_phase_frame = 66;
		}
	} else if(boss_phase_frame < 178) {
		*boss_left_on_back_page += 2;
		if(*boss_left_on_back_page >= 256) {
			boss_phase_frame = 178;
		}
	} else if(boss_phase_frame < 234) {
		*boss_left_on_back_page -= 2;
		if(*boss_left_on_back_page <= 144) {
			boss_phase_frame = 234;
		}
	} else {
		boss_phase_frame = 0;
		patnum_2064E = 128;
		bullets_set_stack_multiplier(1);
	}
	if((boss_phase_frame % 32) == 0) {
		bullets_add_pellet(
			word_26C58,
			word_26C60,
			(randring2_next8_and(0x0F) + 0x38),
			byte_2066E,
			to_sp(3.75f)
		);
		bullets_add_pellet(
			word_26C58, word_26C60, 0x00, byte_2066F[byte_26CC0], to_sp(2.5f)
		);
	}
}

void near mima_181B3(void)
{
	unsigned char angle;
	unsigned char angle_step;
	screen_y_t y;
	int i;
	screen_x_t x;

	if(boss_phase_frame < 20) {
		return;
	}
	if(boss_phase_frame < 70) {
		patnum_2064E = 128;
		mima_move_toward_player();
	} else if(boss_phase_frame == 70) {
		snd_se_play(9);
		patnum_2064E = 131;
		byte_26CC7 = 0x30;
		byte_26CC8 = 0;
		byte_26CC9 = 100;
	} else if(boss_phase_frame < 170) {
		grcg_setcolor(GC_RMW, 13);
		angle_step = (0x40 - byte_26CC8);
		angle_step >>= 2;
		for(i = 0, angle = byte_26CC8; i < 9; angle += angle_step, i++) {
			x = polar_x_fast(word_26C58, 400, angle);
			y = polar_y_fast(word_26C60, 400, angle);
			grcg_line(word_26C58, word_26C60, x, y);
		}
		if(boss_phase_frame >= 36) {
			byte_26CC8++;
		}
		grcg_off();
		if(boss_phase_frame & 1) {
			byte_26CC9++;
		}
		PaletteTone = byte_26CC9;
		palette_show();
	} else if(boss_phase_frame == 170) {
		snd_se_play(5);
		patnum_2064E = 134;
	} else if(boss_phase_frame <= 198) {
		*boss_top_on_back_page = (272 - ((boss_phase_frame & 3) * 60));
		grcg_setcolor(GC_RMW, 13);
		angle = randring2_next8_and(0x7F);
		x = (angle + *boss_left_on_back_page + 8);
		grcg_line(x, PLAYFIELD_TOP, x, (PLAYFIELD_BOTTOM - 1));
		grcg_off();
		if(
			((*boss_left_on_back_page + 16) < player_topleft.x) &&
			((*boss_left_on_back_page + 96) > player_topleft.x)
		) {
			player_is_hit = true;
		}
		if(boss_phase_frame & 0) { // ZUN bug
			byte_26CC9 = 150;
		} else {
			byte_26CC9 = 100;
		}
		PaletteTone = byte_26CC9;
		palette_show();
	} else if(boss_phase_frame <= 262) {
		patnum_2064E = 128;
		*boss_top_on_back_page -= 4;
		if(*boss_top_on_back_page <= 64) {
			boss_phase_frame = 263;
		}
		if((rank != RANK_EASY) && ((boss_phase_frame % 16) == 0)) {
			// ZUN bloat: Reusing [angle] for the group.
			if(byte_26CC0 == 0) {
				angle = BG_16_RING;
			} else {
				angle = BG_32_RING;
			}
			bullets_add_pellet(left_26C56, top_26C5E, 0x00, angle, to_sp(3.75f));
		}
	} else {
		boss_phase_frame = 0;
	}
}

void pascal near mima_183D0(int &velocity_x, int &velocity_y)
{
	int i;
	int j;
	unsigned char angle;
	screen_x_t left;
	screen_y_t top;

	if(boss_phase_frame == 40) {
		snd_se_play(9);
		patnum_2064E = 131;
		word_26CCC = (*boss_left_on_back_page + 80);
		word_26CCE = (*boss_top_on_back_page + 64);
		word_26CCC <<= 4;
		word_26CCE <<= 4;
		word_26CD0 = 4;
		byte_26CD3 = -1;
		byte_26CD4 = 0;
	} else if(boss_phase_frame < 80) {
		return;
	} else if(boss_phase_frame == 80) {
		snd_se_play(10);
		patnum_2064E = 134;
		for(i = 0, angle = angle_26CD2; i < SPHERE_COUNT; i++, angle += 0x40) {
			mima_sphere_left[0][i] = mima_sphere_left[1][i] = polar_x_fast(
				(word_26CCC >> 4), word_26CD0, angle
			);
			mima_sphere_top[0][i] = mima_sphere_top[1][i] = polar_y_fast(
				(word_26CCE >> 4), word_26CD0, angle
			);
			mima_sphere_active[i] = 1;
		}
	} else if(boss_phase_frame < 96) {
		word_26CD0 += 4;
		for(i = 0, angle = angle_26CD2; i < SPHERE_COUNT; i++, angle += 0x40) {
			mima_sphere_left[page_back][i] = polar_x_fast(
				(word_26CCC >> 4), word_26CD0, angle
			);
			mima_sphere_top[page_back][i] = polar_y_fast(
				(word_26CCE >> 4), word_26CD0, angle
			);
		}
	} else {
		word_26CCC += velocity_x;
		word_26CCE += velocity_y;
		word_26CD0 += byte_26CD3;
		if(byte_26CCA == 0) {
			if(byte_26CC0 == 0) {
				goto radius_bounce;
			}
			if(word_26CD0 == 0xC4) {
				boss_phase_frame = 0;
			}
			if(word_26CD0 < 0x14) {
				byte_26CD4 = 0;
				snd_se_play(3);
				bullets_set_stack_multiplier(0);
				for(i = 0; i < SPHERE_COUNT; i++) {
					left = (mima_sphere_left[page_back][i] + 12);
					top = (mima_sphere_top[page_back][i] + 12);
					for(
						j = 0, angle = ((i * 0x40) + angle_26CD2 - 0x40);
						j < 16;
						j++, angle += 8
					) {
						bullets_add_pellet(
							left, top, angle, BG_1, to_sp(2.1875f)
						);
					}
					for(
						j = 0, angle = ((i * 0x40) + angle_26CD2 - 0x3C);
						j < 16;
						j++, angle += 8
					) {
						bullets_add_pellet(
							left, top, angle, BG_1, (to_sp(2.1875f) - j)
						);
					}
					mima_sphere_active[i] = 2;
				}
				bullets_set_stack_multiplier(1);
				word_26CD0 = 0x140;
				patnum_2064E = 128;
			}
		} else {
			if(byte_26CD4 != 0) {
				if(byte_26CC0 != 0) {
					angle_26CD2 += -3;
				} else {
					angle_26CD2 += -5;
				}
			}
			if(word_26CD0 >= 400) {
				goto remove;
			}
			if(word_26CD0 < 0x14) {
				velocity_x = 0;
				velocity_y = 0;
				byte_26CD4 = 1;
				snd_se_play(3);
				angle = (angle_26CD2 + 8);
				for(i = 0; i < SPHERE_COUNT; i++, angle += 0x40) {
					left = (mima_sphere_left[page_back][i] + 12);
					top = (mima_sphere_top[page_back][i] + 12);
					for(j = 0; j < 48; j += 12) {
						bullets_add_pellet(
							left, top, (angle + j), BG_1, (j + to_sp(2.1875f))
						);
					}
				}
				patnum_2064E = 128;
				byte_26CD3 = 5;
			}
		}
		if(0) {
radius_bounce:
			if(word_26CD0 < 32) {
				byte_26CD3 = 2;
			} else if(word_26CD0 > 128) {
				byte_26CD3 = -2;
			}
		}
		left = (word_26CCC >> 4);
		top = (word_26CCE >> 4);
		if(
			((left + word_26CD0) < 0) ||
			((left - word_26CD0) > 0x1A0) ||
			((top + word_26CD0) < -16) ||
			((top - word_26CD0) > 0x182) ||
			(boss_phase_frame > 500)
		) {
remove:
			for(i = 0; i < SPHERE_COUNT; i++) {
				mima_sphere_active[i] = 2;
			}
			boss_phase_frame = 0;
			patnum_2064E = 128;
		}
		for(i = 0, angle = angle_26CD2; i < SPHERE_COUNT; i++, angle += 0x40) {
			mima_sphere_left[page_back][i] = polar_x_fast(
				(word_26CCC >> 4), word_26CD0, angle
			);
			mima_sphere_top[page_back][i] = polar_y_fast(
				(word_26CCE >> 4), word_26CD0, angle
			);
		}
	}
	angle_26CD2 += 5;
}

void near mima_188AA(void)
{
	screen_x_t left;
	screen_y_t top;

	if(boss_phase_frame < 40) {
		return;
	}
	if(boss_phase_frame == 40) {
		byte_26CCA = 0;
		left = (*boss_left_on_back_page + 80);
		top = (*boss_top_on_back_page + 64);
		vector2_between_plus(
			left,
			top,
			player_topleft.x,
			player_topleft.y,
			0,
			point_26CD6.x,
			point_26CD6.y,
			48
		);
	}
	mima_183D0(point_26CD6.x, point_26CD6.y);
}

void near mima_18905(void)
{
	if(boss_phase_frame < 10) {
		return;
	}
	if(boss_phase_frame < 70) {
		patnum_2064E = 128;
		mima_move_toward_player();
	} else if(boss_phase_frame == 70) {
		snd_se_play(9);
		patnum_2064E = 131;
		speed_26CDB = to_sp(1.25f);
	} else if(boss_phase_frame < 100) {
		return;
	} else if(boss_phase_frame == 100) {
		snd_se_play(10);
		patnum_2064E = 134;
		angle_26CDA = randring2_next8();
	} else if(boss_phase_frame <= 160) {
		angle_26CDA--;
	} else if(boss_phase_frame <= 240) {
		angle_26CDA++;
		speed_26CDB++;
	} else if(byte_26CC0 != 0) {
		if(boss_phase_frame <= 320) {
			angle_26CDA++;
			speed_26CDB--;
		} else if(boss_phase_frame <= 400) {
			angle_26CDA--;
			speed_26CDB++;
		} else {
			boss_phase_frame = 0;
			patnum_2064E = 128;
		}
	} else {
		boss_phase_frame = 0;
		patnum_2064E = 128;
	}
	if(
		(boss_phase_frame > 100) &&
		((boss_phase_frame % (8 - rank)) == 0)
	) {
		bullets_add_pellet(
			left_26C5A, top_26C62, angle_26CDA, byte_20671, speed_26CDB
		);
		snd_se_play(3);
	}
}

void near mima_18A1B(void)
{
	if(boss_phase_frame < 10) {
		return;
	}
	if(boss_phase_frame < 70) {
		patnum_2064E = 128;
		mima_move_toward_player();
	} else if(boss_phase_frame == 70) {
		snd_se_play(9);
		patnum_2064E = 131;
		angle_26CDC = -0x28;
	} else if(boss_phase_frame < 100) {
		return;
	} else if(boss_phase_frame == 100) {
		snd_se_play(10);
		patnum_2064E = 134;
	} else if(boss_phase_frame <= 132) {
		bullets_add_pellet(
			left_26C5A,
			top_26C62,
			angle_26CDC,
			BG_2_SPREAD_HORIZONTALLY_SYMMETRIC,
			to_sp(5.625f)
		);
		angle_26CDC += 3;
	} else if(boss_phase_frame <= 150) {
		bullets_add_pellet(
			left_26C5A,
			top_26C62,
			angle_26CDC,
			BG_2_SPREAD_HORIZONTALLY_SYMMETRIC,
			to_sp(5.625f)
		);
		angle_26CDC += -3;
	} else if(boss_phase_frame <= 226) {
		if((boss_phase_frame % 8) == 1) {
			if(byte_26CC0 == 0) {
				bullets_add_pellet(
					left_26C5A,
					top_26C62,
					0x40,
					BG_3_SPREAD_NARROW,
					to_sp(5.625f)
				);
			} else {
				bullets_add_pellet(
					left_26C5A,
					top_26C62,
					(randring2_next8_and(0x3F) + 0x20),
					BG_1,
					to_sp(5.0f)
				);
				bullets_add_pellet(
					left_26C5A,
					top_26C62,
					(randring2_next8_and(0x3F) + 0x20),
					BG_1,
					to_sp(5.0f)
				);
			}
		}
	} else {
		boss_phase_frame = 0;
		patnum_2064E = 128;
	}
}

void near mima_18B4B(void)
{
	screen_x_t left;
	screen_y_t top;

	if(boss_phase_frame < 40) {
		return;
	}
	if(boss_phase_frame == 40) {
		byte_26CCA = 1;
		left = (*boss_left_on_back_page + 80);
		top = (*boss_top_on_back_page + 64);
		vector2_between_plus(
			left,
			top,
			player_topleft.x,
			player_topleft.y,
			0,
			point_26CDE.x,
			point_26CDE.y,
			52
		);
	}
	mima_183D0(point_26CDE.x, point_26CDE.y);
}

void near mima_18BA6(void)
{
	if(boss_phase_frame < 10) {
		return;
	}
	if(boss_phase_frame == 10) {
		snd_se_play(9);
		patnum_2064E = 131;
	} else if(boss_phase_frame < 40) {
		return;
	} else if(boss_phase_frame == 40) {
		snd_se_play(10);
		patnum_2064E = 134;
		if(word_26C68 == 6) {
			items_add(left_26C5A, top_26C62, IT_BOMB);
		} else {
			items_add(left_26C5A, top_26C62, IT_BIGPOWER);
		}
	} else if(boss_phase_frame == 60) {
		snd_se_play(10);
		items_add(left_26C5A, top_26C62, IT_BIGPOWER);
	} else if(boss_phase_frame == 80) {
		snd_se_play(10);
		patnum_2064E = 128;
		if(word_26C68 == 2) {
			items_add(left_26C5A, top_26C62, IT_1UP);
		} else {
			items_add(left_26C5A, top_26C62, IT_BIGPOWER);
		}
	}
}

#define mima_star_add(angle_delta) { \
	bullets_add_16x16( \
		left_26C5A, \
		top_26C62, \
		(angle_26CE2 + angle_delta), \
		BSM_BOUNCE_TOP_BOTTOM, \
		PAT_BULLET16_STAR, \
		to_sp(5.0f) \
	); \
}

void near mima_18C4A(void)
{
	if(boss_phase_frame < 10) {
		return;
	}
	if(boss_phase_frame < 70) {
		patnum_2064E = 128;
		mima_move_toward_player();
	} else if(boss_phase_frame == 70) {
		snd_se_play(9);
		patnum_2064E = 131;
	} else if(boss_phase_frame < 100) {
		return;
	} else if(boss_phase_frame == 100) {
		snd_se_play(10);
		patnum_2064E = 134;
		angle_26CE2 = iatan2(
			(player_topleft.y - top_26C62), (player_topleft.x - left_26C5A)
		);
		direction_26CE3 = (player_topleft.x <= (
			PLAYFIELD_LEFT + (PLAYFIELD_W / 2) - (PLAYER_W / 2)
		)) ? -1 : 1;
		bullet_special.u3.turns_max = byte_26CC0;
	} else if(boss_phase_frame <= 300) {
		if((boss_phase_frame % 8) == 0) {
			snd_se_play(10);
			if(rank != RANK_EASY) {
				mima_star_add(+0x0A);
				mima_star_add(-0x0A);
				mima_star_add(+0x1E);
				mima_star_add(-0x1E);
			} else {
				mima_star_add(+0x0F);
				mima_star_add(-0x0F);
			}
		}
		if((boss_phase_frame >= 150) && (boss_phase_frame & 1)) {
			if(boss_phase_frame < 210) {
				angle_26CE2 += direction_26CE3;
			} else {
				angle_26CE2 -= direction_26CE3;
			}
		}
	} else {
		boss_phase_frame = 0;
		patnum_2064E = 128;
	}
}

void near mima_18DE0(void)
{
	if(boss_phase_frame < 30) {
		return;
	}
	if(boss_phase_frame < 70) {
		patnum_2064E = 128;
		mima_move_toward_player();
	} else if(boss_phase_frame == 70) {
		snd_se_play(9);
		patnum_2064E = 131;
	} else if(boss_phase_frame < 120) {
		return;
	} else if(boss_phase_frame == 120) {
		snd_se_play(3);
		patnum_2064E = 134;
		angle_26CE4 = 0x04;
		bullets_add_pellet(
			left_26C5A, top_26C62, (angle_26CE4 + 8), BG_3_SPREAD_NARROW, to_sp(5.0f)
		);
		bullets_add_pellet(
			left_26C5A, top_26C62, (0x78 - angle_26CE4), BG_3_SPREAD_NARROW, to_sp(5.0f)
		);
		angle_26CE4 += 10;
	} else if(boss_phase_frame <= 160) {
		if((boss_phase_frame % 10) == 0) {
			snd_se_play(3);
			bullets_add_pellet(
				left_26C5A, top_26C62, (angle_26CE4 + 8), BG_3_SPREAD_NARROW, to_sp(5.0f)
			);
			bullets_add_pellet(
				left_26C5A, top_26C62, (0x78 - angle_26CE4), BG_3_SPREAD_NARROW, to_sp(5.0f)
			);
			angle_26CE4 += 10;
		}
	} else {
		boss_phase_frame = 0;
		patnum_2064E = 128;
	}
}

void near mima_18EB8(void)
{
	int i;
	unsigned char pellet_angle;
	unsigned char angle;

	if(boss_phase_frame < 40) {
		return;
	}
	if(boss_phase_frame == 40) {
		snd_se_play(9);
		patnum_2064E = 131;
		word_26CE6 = (*boss_left_on_back_page + 80);
		word_26CE8 = (*boss_top_on_back_page + 80);
		word_26CEA = 4;
		angle_26CEC = randring2_next8();
	} else if(boss_phase_frame < 80) {
		return;
	} else if(boss_phase_frame == 80) {
		snd_se_play(10);
		patnum_2064E = 134;
		for(i = 0, angle = angle_26CEC; i < SPHERE_COUNT; i++, angle += 0x40) {
			mima_sphere_left[0][i] = mima_sphere_left[1][i] = polar_x_fast(
				(word_26CE6 >> 4), word_26CEA, angle
			);
			mima_sphere_top[0][i] = mima_sphere_top[1][i] = polar_y_fast(
				(word_26CE8 >> 4), word_26CEA, angle
			);
			mima_sphere_active[i] = 1;
		}
	} else {
		word_26CEA += 2;
		for(i = 0, angle = angle_26CEC; i < SPHERE_COUNT; i++, angle += 0x40) {
			mima_sphere_left[page_back][i] = polar_x_fast(
				word_26CE6, word_26CEA, angle
			);
			mima_sphere_top[page_back][i] = polar_y_fast(
				word_26CE8, word_26CEA, angle
			);
			if(
				((180 - (rank * 16)) < word_26CEA) &&
				((boss_phase_frame & 3) == 0)
			) {
				snd_se_play(3);
				if(
					(mima_sphere_left[page_back][i] > 0) &&
					(mima_sphere_left[page_back][i] < 0x1A0) &&
					(mima_sphere_top[page_back][i] > 0) &&
					(mima_sphere_top[page_back][i] < 0x190)
				) {
					pellet_angle = (byte_26CC0 == 0)
						? ((i * 0x40) + angle_26CEC + 0x40)
						: ((i * 0x40) + angle_26CEC + 0x60);
					bullets_add_pellet(
						mima_sphere_left[page_back][i],
						mima_sphere_top[page_back][i],
						pellet_angle,
						BG_1,
						to_sp(8.0f)
					);
				}
			}
		}
		if(word_26CEA > 0x1CC) {
			for(i = 0; i < SPHERE_COUNT; i++) {
				mima_sphere_active[i] = 0;
			}
			boss_phase_frame = 0;
			patnum_2064E = 128;
		}
	}
	angle_26CEC += -3;
}

void near mima_19173(void)
{
	if(boss_phase_frame < 50) {
		return;
	}
	if(boss_phase_frame == 50) {
		angle_26CED = 0;
	} else if(boss_phase_frame < 150) {
		if(boss_phase_frame & 1) {
			snd_se_play(3);
			bullets_add_16x16(
				x_26C5C,
				y_26C64,
				angle_26CED,
				BG_2_SPREAD_HORIZONTALLY_SYMMETRIC,
				PAT_BULLET16_BALL,
				to_sp(3.125f)
			);
			angle_26CED += byte_20672;
		}
	} else {
		boss_phase_frame = 0;
	}
}

#define mima_circles_put(col) { \
	grcg_setcolor(GC_RMW, col); \
	grcg_circle(x_26C5C, y_26C64, byte_26CEF); \
	grcg_circle(x_26C5C, y_26C64, (byte_26CEF * 2)); \
	grcg_circle(x_26C5C, y_26C64, (byte_26CEF * 4)); \
}

#define mima_random_pair_add() { \
	bullets_add_16x16( \
		x_26C5C, \
		y_26C64, \
		2, \
		BG_RANDOM_ANGLE_AND_SPEED, \
		PAT_BULLET16_BALL, \
		to_sp(3.75f) \
	); \
	bullets_add_pellet( \
		x_26C5C, y_26C64, 2, BG_RANDOM_ANGLE_AND_SPEED, to_sp(3.75f) \
	); \
}

void near mima_191CC(void)
{
	if(boss_phase_frame < 50) {
		return;
	}
	if(boss_phase_frame == 50) {
		angle_26CEE = iatan2(
			((player_topleft.y + 12) - y_26C64),
			((player_topleft.x + 12) - x_26C5C)
		);
		byte_26CEF = 30;
		snd_se_play(9);
	} else if(boss_phase_frame <= 80) {
		if(boss_phase_frame & 1) {
			mima_circles_put(0);
			byte_26CEF += -2;
			if(boss_phase_frame != 80) {
				mima_circles_put(13);
				grcg_off();
			}
		}
	} else if(boss_phase_frame < 200) {
		if((boss_phase_frame & 7) == 0) {
			mima_random_pair_add();
		}
		if((rank > RANK_EASY) && ((boss_phase_frame & 7) == 4)) {
			mima_random_pair_add();
		}
		if((boss_phase_frame & 7) == 0) {
			snd_se_play(3);
		}
	} else {
		boss_phase_frame = 0;
	}
}

void near mima_19353(void)
{
	if(boss_phase_frame < 50) {
		return;
	}
	if(boss_phase_frame < 190) {
		if((boss_phase_frame & 0x1F) == 0) {
			bullets_add_16x16(
				x_26C5C,
				y_26C64,
				0,
				BG_5_SPREAD_MEDIUM_AIMED,
				PAT_BULLET16_BALL,
				to_sp(5.625f)
			);
		} else if((boss_phase_frame & 0x1F) == 16) {
			bullets_add_16x16(
				x_26C5C,
				y_26C64,
				0,
				BG_4_SPREAD_MEDIUM_AIMED,
				PAT_BULLET16_BALL,
				to_sp(5.625f)
			);
		}
	} else {
		boss_phase_frame = 0;
	}
}

void near mima_193A4(void)
{
	int pattern;

	if(boss_phase_frame == 0) {
		if((boss_damage >= word_26CF0) || (word_26CBE > word_26CF2)) {
			word_26C68++;
			word_26C66 = 0;
			word_26CBE = 0;
		} else {
			word_26CBE++;
			if(word_26CBE >= word_26CF6) {
				word_26C66 = 1;
			}
			if(byte_26CC0 == 0) {
				pattern = (word_26C6A + 1);
				if(pattern >= word_26CF4) {
					pattern = 0;
				}
			} else {
				pattern = (word_26C6A + 1);
				if(pattern > 7) {
					pattern = 0;
				}
			}
			word_26C6A = pattern;
			if(*boss_top_on_back_page < 48) {
				word_1EE56 = 1;
			} else if(*boss_top_on_back_page > 64) {
				word_1EE56 = -1;
			} else {
				word_1EE56 = (1 - (randring2_next8() % 3));
			}
		}
	}
	if(boss_phase_frame < 10) {
		*boss_top_on_back_page += word_1EE56;
	}
}

#define mima_palette_set(r_, g_, b_) { \
	Palettes[0].c.r = r_; \
	Palettes[0].c.g = g_; \
	Palettes[0].c.b = b_; \
	palette_show(); \
}

int mima_update(void)
{
	unsigned char radius;
	int i;

	boss_phase_frame++;
	shot_target_left = (*boss_left_on_back_page + 72);
	shot_target_top = (*boss_top_on_back_page + 56);
	left_26C56 = (*boss_left_on_back_page + 32);
	word_26C58 = (*boss_left_on_back_page + 40);
	left_26C5A = (*boss_left_on_back_page + 64);
	x_26C5C = (*boss_left_on_back_page + 64);
	top_26C5E = (*boss_top_on_back_page + 96);
	word_26C60 = (*boss_top_on_back_page + 16);
	top_26C62 = (*boss_top_on_back_page + 114);
	y_26C64 = (*boss_top_on_back_page + 44);

	if((stage_frame & 1) == 0) {
		if(!reduce_effects) {
			byte_26CF8 += 8;
			grcg_setcolor(GC_RMW, 3);
			for(i = 0, radius = byte_26CF8; i < 2; i++, radius += 0x80) {
				circle_dots_put(224, 200, radius, 16);
			}
			grcg_off();
		}
		if(word_26C68 & 1) {
			byte_26CFA++;
			if(byte_26CFA >= 0x40) {
				byte_26CFA = 0;
			}
			byte_26CC2 = (byte_26CF9 - (byte_26CFA / 4));
		}
	}

	if(word_26C68 == 0) {
		byte_26CC1 = (boss_phase_frame / 2);
		byte_26CC2 = (boss_phase_frame / 3);
		mima_palette_set((boss_phase_frame >> 1), 0, (boss_phase_frame >> 1));
		if(boss_phase_frame > 100) {
			byte_26CF9 = byte_26CC2;
			byte_26CFA = 0;
			word_26C68 = 1;
			boss_phase_frame = 0;
			word_26C6A = (randring2_next8() % 3);
			word_26CBE = 0;
			word_26CF0 = 600;
			word_26CF6 = 2;
			word_26CF2 = 10;
			word_26CF4 = 3;
			boss_damage = 0;
		}
	} else if(word_26C68 == 1) {
		switch(word_26C6A) {
		case 0:	mima_180EC();	break;
		case 1:	mima_181B3();	break;
		case 2:	mima_188AA();	break;
		}
		mima_193A4();
	} else if(word_26C68 == 2) {
		byte_26CC1 = ((boss_phase_frame / 4) + 50);
		byte_26CC2 = ((boss_phase_frame / 5) + 30);
		mima_18BA6();
		mima_palette_set(
			(51 - (boss_phase_frame >> 1)), (boss_phase_frame >> 1), 50
		);
		if(boss_phase_frame > 100) {
			word_26C68 = 3;
			boss_phase_frame = 0;
			word_26C6A = (randring2_next8() % 3);
			word_26CBE = 0;
			byte_26CF9 = byte_26CC2;
			word_26CF0 = 700;
			goto phase_init;
		}
	} else if(word_26C68 == 3) {
		switch(word_26C6A) {
		case 0:	mima_18905();	break;
		case 1:	mima_18A1B();	break;
		case 2:	mima_18B4B();	break;
		}
		mima_193A4();
	} else if(word_26C68 == 4) {
		byte_26CC1 = ((boss_phase_frame / 4) + 75);
		byte_26CC2 = ((boss_phase_frame / 5) + 50);
		mima_18BA6();
		mima_palette_set(
			(boss_phase_frame >> 1),
			(51 - (boss_phase_frame >> 1)),
			(51 - (boss_phase_frame >> 1))
		);
		if(boss_phase_frame > 100) {
			word_26C68 = 5;
			boss_phase_frame = 0;
			word_26C6A = (randring2_next8() % 3);
			byte_26CF9 = byte_26CC2;
			word_26CBE = 0;
			word_26CF0 = 800;
phase_init:
			word_26CF6 = 2;
			word_26CF2 = 12;
			word_26CF4 = 3;
			boss_damage = 0;
		}
	} else if(word_26C68 == 5) {
		switch(word_26C6A) {
		case 0:	mima_18C4A();	break;
		case 1:	mima_18DE0();	break;
		case 2:	mima_18EB8();	break;
		}
		mima_193A4();
	} else if(word_26C68 == 6) {
		byte_26CC1 = ((boss_phase_frame / 4) + 100);
		byte_26CC2 = ((boss_phase_frame / 5) + 70);
		mima_18BA6();
		mima_palette_set((51 - (boss_phase_frame >> 1)), 0, 0);
		if(boss_phase_frame > 100) {
			byte_26CF9 = byte_26CC2;
			word_26C68 = 7;
			boss_phase_frame = 0;
			word_26C6A = randring2_next8_and(7);
			word_26CBE = 0;
			word_26CF0 = 1500;
			word_26CF6 = 3;
			word_26CF2 = 200;
			word_26CF4 = 9;
			byte_26CC0 = 1;
			boss_damage = 0;
			byte_26CC4 = 2;
		}
	} else if(word_26C68 == 7) {
		switch(word_26C6A) {
		case 0:	mima_18C4A();	break;
		case 1:	mima_188AA();	break;
		case 2:	mima_18905();	break;
		case 3:	mima_181B3();	break;
		case 4:	mima_18A1B();	break;
		case 5:	mima_18B4B();	break;
		case 6:	mima_180EC();	break;
		case 7:	mima_18DE0();	break;
		case 8:	mima_18EB8();	break;
		}
		mima_193A4();
	} else if(word_26C68 == 9) {
		switch(word_26C6A) {
		case 0:	mima_19173();	break;
		case 1:	mima_191CC();	break;
		case 2:	mima_19353();	break;
		}
		mima_193A4();
		if(boss_phase_frame == 1) {
			*boss_top_on_back_page = 64;
		}
		if(boss_phase_frame < 30) {
			*boss_left_on_back_page += ((x_26C5C < player_topleft.x) ? 1 : -1);
		}
	}
	mima_17C92();
	mima_17F27();
	mima_17D59();
	if(word_26C68 == 8) {
		if(mima_19C8D() == 0) {
			word_26C68++;
			patnum_2064E = 128;
			word_26CBE = 0;
			boss_phase_frame = 0;
			word_26C6A = (randring2_next8() % 3);
			word_26CBE = 0;
			boss_damage = 0;
			word_26CF0 = 1100;
			word_26CF6 = 2;
			word_26CF2 = 200;
			word_26CF4 = 3;
			byte_26CC5 = 1;
			byte_26CC4 = 2;
			byte_26CC1 = 200;
			byte_26CC2 = 150;
			byte_26CF9 = byte_26CC2;
		}
	} else if(word_26C68 == 10) {
		mima_end();
	}
	return 1;
}
