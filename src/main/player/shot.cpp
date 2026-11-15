/* ReC98
 * -----
 * Scroll reset, and the player's shot types, of TH02's MAIN.EXE
 */

#pragma option -zCmain_01_TEXT -zPmain_01 -G -a2

#include "platform.h"
#include "x86real.h"
#include "pc98.h"
#include "libs/master.lib/master.hpp"
#include "libs/master.lib/pc98_gfx.hpp"
#include "th02/resident.hpp"
#include "th02/hardware/pages.hpp"
#include "th02/math/randring.hpp"
#include "th02/main/frames.hpp"
#include "th02/main/scroll.hpp"
#include "th02/main/player/player.hpp"
#include "th02/main/player/shot.hpp"
#include "th02/main/tile/tile.hpp"

extern int scroll_unused;
extern vram_offset_t scroll_line_vo;
extern int tile_rows_scrolled;
extern int16_t tile_row_scrolled;
uint8_t scroll_interval = 4;
bool scroll_done = false;
uint8_t byte_1E502 = 0;
uint8_t scroll_speed_this_frame = 0;
extern uint8_t byte_2034E;

// Uninitialized data, in the original order
// -----------------------------------------

pixel_length_8_t scroll_speed;
uint8_t scroll_cycle;
vram_y_t scroll_line;
int scroll_unused;
vram_offset_t scroll_line_vo;
int tile_rows_scrolled;
int16_t tile_row_scrolled;
uint8_t byte_2034E;
uint8_t byte_2034F; // ZUN bloat: Unused.
uint8_t shot_c_cycle;
uint8_t shot_frames[SHOT_COUNT];
shot_t shots[SHOT_COUNT];
int shot_target_left;
int shot_target_top;
int shot_target_left_unused;
uint8_t option_shots_active;
uint8_t byte_205DF;
uint8_t byte_205E0;
int shot_slot;
int shot_top_new;
screen_x_t player_left_on_page[PAGE_COUNT];
screen_y_t player_top_on_page[PAGE_COUNT];
screen_x_t near* player_left_on_back_page;
screen_y_t near* player_top_on_back_page;
screen_x_t near* player_option_left_left_on_back_page;
screen_y_t near* player_option_left_top_on_back_page;
screen_point_t player_topleft;
screen_point_t player_option_left_topleft[PAGE_COUNT];
void (near *playchar_shot_func)(void);
player_hit_t player_is_hit;
unsigned char player_invincibility_time;
bool player_invincible_via_bomb;
bool quit;
uint8_t stage_miss_count;
uint8_t miss_frame;

// All values are signed. (Yes, allowing you to invert the controls with
// negative values!)
int8_t playchar_speed_aligned_x;
int8_t playchar_speed_aligned_y;
int8_t playchar_speed_diagonal_x;
int8_t playchar_speed_diagonal_y;

uint8_t shot_max_param_1;
uint8_t shot_max_param_2;
int8_t shot_max_spread;
uint8_t byte_20611;
uint32_t stage_frame;
int midboss_tile_row;
page_t page_back;
page_t page_front;
// -----------------------------------------

void sub_CA1C(void)
{
	scroll_speed = 1;
	scroll_cycle = 0;
	scroll_line = 0;
	scroll_unused = 0;
	tile_rows_scrolled = 0;
	scroll_interval = 4;
	scroll_done = false;
	byte_2034E = 0;
	byte_1E502 = 0;
	scroll_line_vo = 0;
	tile_row_scrolled = 0;
	scroll_speed_this_frame = 0;
}

extern int shot_slot;
extern int shot_top_new;
extern uint8_t shot_param_1;
extern uint8_t shot_param_2;
extern int8_t shot_max_spread;
extern uint8_t option_shots_active;
extern uint8_t shot_c_cycle;
extern uint8_t shot_frames[SHOT_COUNT];
extern int8_t shot_option_frames_per_age;
extern int shot_target_left;
extern int shot_target_top;
extern int shot_target_left_unused;
extern const int SHOT_BIG_HIT_PATNUMS[] = {
	0x27, 0x27, 0x27, 0x28, 0x29, 0x2F
};

// ZUN set each of these byte pairs with a single 16-bit store.
#define shot_byte_pair_set(p, field, lo, hi) ( \
	*reinterpret_cast<uint16_t near *>(&(p)->field) = ((lo) + ((hi) << 8)) \
)

void pascal near shot_add(int offset_left, unsigned char angle)
{
	register shot_t near *p = &shots[shot_slot];
	int page = (page_back * 2);

	shot_byte_pair_set(p, flag, SF_ALIVE, 0);
	(&p->pos[0].x)[page] = ((player_topleft.x + offset_left) << 4);
	(&p->pos[0].y)[page] = shot_top_new;
	p->velocity.x = ((CosTable8[angle] * 12) >> 4);
	p->velocity.y = ((SinTable8[angle] * 12) >> 4);
	shot_byte_pair_set(p, patnum, shot_param_1, false);
}

void pascal near option_shot_add(
	int offset_left, int velocity_x, int velocity_y
)
{
	register shot_t near *p = &shots[shot_slot];
	int page = (page_back * 2);

	shot_byte_pair_set(p, flag, SF_ALIVE, 1);
	(&p->pos[0].x)[page] = (
		(*player_option_left_left_on_back_page + offset_left) << 4
	);
	(&p->pos[0].y)[page] = (*player_option_left_top_on_back_page << 4);
	p->velocity.x = velocity_x;
	p->velocity.y = velocity_y;
	shot_byte_pair_set(p, patnum, shot_param_2, true);
}

void near shot_a(void)
{
	int i = 0;
	int last = 0;
	int tmp = 0;

	shot_top_new = ((player_topleft.y + 32) << 4);
	for(shot_slot = 0; shot_slot < SHOT_COUNT; shot_slot++) {
		if(shots[shot_slot].flag != SF_FREE) {
			continue;
		}
		switch(shot_level) {
		case 0:
			shot_add(8, 0xC0);
			break;
		case 1:
			shot_add(8, (randring1_next8_and(3) + 0xBE));
			break;
		case 2:
			if(i == 1) {
				tmp = 16;
			}
			shot_add(tmp, (randring1_next8_and(7) + 0xBC));
			last = 1;
			break;
		case 3:
			if(i == 1) {
				tmp = 16;
			}
			shot_add(tmp, 0xC0);
			last = 1;
			break;
		case 5:
			tmp = 2;
			// fallthrough
		case 4:
			if(i == 0) {
				shot_add(8, 0xC0);
			} else if(i == 1) {
				shot_add(8, (0xBB - tmp));
			} else {
				shot_add(8, (tmp + 0xC5));
			}
			last = 2;
			break;
		case 6:
			if(i == 0) {
				shot_add(0, 0xC0);
			} else if(i == 1) {
				shot_add(16, 0xC0);
			} else {
				tmp = (randring1_next8_and(0x1F) + 0xB1);
				shot_add(8, tmp);
			}
			last = 3;
			break;
		case 7:
			if(i == 0) {
				shot_add(0, 0xC0);
			} else if(i == 1) {
				shot_add(16, 0xC0);
			} else if(i == 2) {
				shot_add(8, 0xB8);
			} else {
				shot_add(8, 0xC8);
			}
			last = 3;
			break;
		case 8:
			if(i == 0) {
				tmp = 0xC0;
			} else if(i == 1) {
				tmp = 0xB8;
			} else if(i == 2) {
				tmp = 0xC8;
			} else if(i == 3) {
				tmp = 0xB0;
			} else {
				tmp = 0xD0;
			}
			shot_add(8, tmp);
			last = 4;
			break;
		case 9:
			if(i == 0) {
				shot_add(0, 0xC0);
			} else if(i == 1) {
				shot_add(16, 0xC0);
			} else if(i == 2) {
				shot_param_1 = 52;
				tmp = 0xB3;
				shot_add(8, tmp);
			} else if(i == 3) {
				tmp = 0xCD;
				shot_add(8, tmp);
			} else if(i == 4) {
				shot_param_1 = 54;
				tmp = (0xC0 - shot_max_spread);
				shot_add(-24, tmp);
			} else {
				tmp = (shot_max_spread + 0xC0);
				shot_add(40, tmp);
			}
			last = 5;
			break;
		}
		if(i == last) {
			break;
		}
		i++;
	}
	option_shots_active = false;
}

void near shot_b(void)
{
	int i = 0;
	int last = 0;
	int tmp = 0;
	bool main_done = false;

	shot_top_new = ((player_topleft.y + 32) << 4);
	for(shot_slot = 0; shot_slot < SHOT_COUNT; shot_slot++) {
		if(shots[shot_slot].flag != SF_FREE) {
			continue;
		}
		if(!main_done) {
			switch(shot_level) {
			case 0:
				shot_add(8, 0xC0);
				break;
			case 1:
				shot_add(8, (randring1_next8_and(3) + 0xBE));
				break;
			case 2:
				shot_add(8, (randring1_next8_and(7) + 0xBC));
				break;
			case 3:
				if(i == 0) {
					shot_add(0, (randring1_next8_and(7) + 0xBC));
				} else {
					shot_add(16, (randring1_next8_and(7) + 0xBC));
				}
				last = 1;
				break;
			case 4:
				if(i == 1) {
					tmp = 16;
				}
				shot_add(tmp, 0xC0);
				last = 1;
				break;
			case 5:
				if(i == 0) {
					shot_add(0, 0xBE);
				} else {
					shot_add(16, 0xC2);
				}
				last = 1;
				break;
			case 7:
				tmp = 2;
				// fallthrough
			case 6:
				if(i == 0) {
					shot_add(8, 0xC0);
				} else if(i == 1) {
					shot_add(8, (0xBB - tmp));
				} else {
					shot_add(8, (tmp + 0xC5));
				}
				last = 2;
				break;
			case 8:
				if(i == 0) {
					shot_add(0, 0xC0);
				} else if(i == 1) {
					shot_add(16, 0xC0);
				} else {
					tmp = (randring1_next8_and(0x1F) + 0xB1);
					shot_add(8, tmp);
				}
				last = 3;
				break;
			case 9:
				if(i == 0) {
					shot_add(0, 0xC0);
				} else if(i == 1) {
					shot_add(16, 0xC0);
				} else if(i == 2) {
					shot_param_1 = 50;
					if(shot_target_top > 0) {
						tmp = iatan2(
							(shot_target_top - player_topleft.y - 32),
							(shot_target_left - player_topleft.x)
						);
						tmp += (randring1_next8_and(7) - 3);
					} else {
						tmp = 0xB7;
					}
					shot_add(0, tmp);
				} else if(i == 3) {
					if(shot_target_top > 0) {
						tmp = iatan2(
							(shot_target_top - player_topleft.y - 32),
							(shot_target_left - player_topleft.x - 16)
						);
						tmp += (randring1_next8_and(7) - 3);
					} else {
						tmp = 0xC9;
					}
					shot_add(16, tmp);
				}
				last = 3;
				break;
			}
			if(i == last) {
				main_done = true;
				i = 0;
				continue;
			}
		} else {
			if(shot_level < 2) {
				return;
			}
			if(shot_level == SHOT_LEVEL_MAX) {
				shot_target_left_unused = shot_target_left;
			} else {
				shot_target_left_unused = -1;
			}
			if(option_shots_active) {
				return;
			}
			switch(shot_level) {
			case 2:
			case 3:
			case 4:
				option_shot_add((i * 48), 0, -4);
				last = 1;
				break;
			case 5:
			case 6:
			case 7:
				if(i == 0) {
					option_shot_add(0, 0, -16);
				} else if(i == 1) {
					option_shot_add(48, 0, -16);
				} else if(i == 2) {
					option_shot_add(-8, -3, -6);
				} else {
					option_shot_add(56, 3, -6);
				}
				last = 3;
				break;
			case 8:
				if(i == 0) {
					option_shot_add(0, 0, -34);
				} else if(i == 1) {
					option_shot_add(48, 0, -34);
				} else if(i == 2) {
					option_shot_add(-8, -5, -28);
				} else if(i == 3) {
					option_shot_add(56, 5, -28);
				} else if(i == 4) {
					option_shot_add(-16, -10, -24);
				} else {
					option_shot_add(64, 10, -24);
				}
				last = 5;
				// ZUN bug: Missing `break`. The shot is immediately replaced by
				// the respective level 9 one.
			case 9:
				if(i == 0) {
					option_shot_add(0, 0, -54);
				} else if(i == 1) {
					option_shot_add(48, 0, -54);
				} else if(i == 2) {
					option_shot_add(-8, -16, -50);
				} else if(i == 3) {
					option_shot_add(56, 16, -50);
				} else if(i == 4) {
					option_shot_add(-16, -32, -40);
				} else {
					option_shot_add(64, 32, -40);
				}
				last = 5;
				break;
			}
			if(i == last) {
				option_shots_active = true;
				return;
			}
		}
		i++;
	}
}

void near shot_c(void)
{
	int i = 0;
	int last;
	int tmp = 0;
	bool main_done;

	last = 0;
	main_done = false;

	shot_c_cycle++;
	shot_top_new = ((player_topleft.y + 32) << 4);
	if(shot_level >= 2) {
		shot_param_1 = 124;
	}
	for(shot_slot = 0; shot_slot < SHOT_COUNT; shot_slot++) {
		if(shots[shot_slot].flag != SF_FREE) {
			continue;
		}
		if(!main_done) {
			switch(shot_level) {
			case 0:
			case 1:
			case 2:
				shot_add(8, 0xC0);
				break;
			case 3:
			case 4:
			case 5:
				shot_add(8, (randring1_next8_and(3) + 0xBE));
				break;
			case 6:
			case 7:
			case 8:
				shot_add(8, (randring1_next8_and(7) + 0xBC));
				break;
			case 9:
				if(i == 0) {
					shot_add(8, 0xC0);
				} else {
					if(i == 1) {
						tmp = -16;
						shot_param_1 = 49;
					} else if(i == 2) {
						tmp = 32;
					} else if(i == 3) {
						tmp = -16;
					} else if(i == 4) {
						tmp = 32;
					}
					shot_add(tmp, 0xC0);
					if(i >= 3) {
						shots[shot_slot].pos[page_back].y -= (16 << 4);
					}
				}
				if((shot_c_cycle % 3) == 0) {
					last = 4;
				} else {
					last = 0;
				}
				break;
			}
			if(i == last) {
				main_done = true;
				i = 0;
				continue;
			}
		} else {
			if(shot_level < 2) {
				return;
			}
			if(shot_level < 3) {
				if(option_shots_active) {
					return;
				}
			} else if(shot_level < 4) {
				if(shot_c_cycle % 5) {
					return;
				}
			} else if(shot_level < 6) {
				if(shot_c_cycle & 3) {
					return;
				}
			} else if(shot_level < SHOT_LEVEL_MAX) {
				if(shot_c_cycle % 3) {
					return;
				}
			} else {
				if(shot_c_cycle & 1) {
					return;
				}
			}
			switch(shot_level) {
			case 2:
			case 3:
				option_shot_add((i * 48), 0, -4);
				last = 1;
				break;
			case 4:
				option_shot_add((i * 48), 0, -16);
				last = 1;
				break;
			case 5:
				if(i == 0) {
					option_shot_add(0, 4, -16);
				} else if(i == 1) {
					option_shot_add(48, -4, -16);
				} else if(i == 2) {
					option_shot_add(-8, 0, -16);
				} else {
					option_shot_add(56, 0, -16);
				}
				last = 3;
				break;
			case 6:
				if(i == 0) {
					option_shot_add(0, 6, -24);
				} else if(i == 1) {
					option_shot_add(48, -6, -24);
				} else if(i == 2) {
					option_shot_add(-8, 0, -24);
				} else {
					option_shot_add(56, 0, -24);
				}
				last = 3;
				break;
			case 7:
				if(i == 0) {
					option_shot_add(0, 6, -26);
				} else if(i == 1) {
					option_shot_add(48, -6, -26);
				} else if(i == 2) {
					option_shot_add(-8, 0, -26);
				} else {
					option_shot_add(56, 0, -26);
				}
				last = 3;
				break;
			case 8:
				if(i == 0) {
					option_shot_add(0, 10, -48);
				} else if(i == 1) {
					option_shot_add(48, -10, -48);
				} else if(i == 2) {
					option_shot_add(-8, -4, -48);
				} else {
					option_shot_add(56, 4, -48);
				}
				last = 3;
				break;
			case 9:
				if(i == 0) {
					option_shot_add(0, -10, -160);
				} else if(i == 1) {
					option_shot_add(48, 10, -160);
				} else if(i == 2) {
					option_shot_add(-8, 20, -140);
				} else if(i == 3) {
					option_shot_add(56, -20, -140);
				} else if(i == 4) {
					option_shot_add(-16, -7, -120);
				} else {
					option_shot_add(64, 7, -120);
				}
				if((shot_c_cycle & 3) == 0) {
					last = 5;
				} else {
					last = 3;
				}
				break;
			}
			if(i == last) {
				option_shots_active = true;
				return;
			}
		}
		i++;
	}
}

void shots_clear(void)
{
	int i;
	for(i = 0; i < SHOT_COUNT; i++) {
		shots[i].flag = SF_FREE;
	}
}

void near shots_invalidate(void)
{
	screen_x_t left;
	uint8_t option_shots_alive = 0;
	register shot_t near *p = shots;
	register int i = 0;

	for(; i < SHOT_COUNT; i++, p++) {
		if(p->flag == SF_FREE) {
			continue;
		}
		if(!reduce_effects || (page_back == (i & 1))) {
			left = (p->pos[page_back].x >> 4);
			if((p->patnum < 124) || (p->age == 0)) {
				tiles_invalidate_rect(
					left, (p->pos[page_back].y >> 4), 16, 16
				);
			} else {
				tiles_invalidate_rect(
					left, (p->pos[page_back].y >> 4), 32, 32
				);
			}
		}
		if(p->flag == SF_REMOVE) {
			p->flag = SF_FREE;
			continue;
		}
		p->pos[page_back].x = shots[i].pos[page_front].x;
		p->pos[page_back].y = shots[i].pos[page_front].y;
		if(p->from_option) {
			option_shots_alive++;
		}
	}
	if(!option_shots_alive) {
		option_shots_active = false;
	}
}

void near shots_update_and_render(void)
{
	int i;
	screen_x_t left;
	int near *pos_x;
	int near *pos_y;
	uint8_t skip;
	int page_offset;
	register shot_t near *p;
	register vram_y_t top;

	skip = (reduce_effects << page_back);
	page_offset = (page_back * sizeof(screen_point_t));
	for(p = shots, i = 0; i < SHOT_COUNT; i++, skip ^= 3, p++) {
		if(p->flag != SF_ALIVE) {
			continue;
		}
		pos_x = reinterpret_cast<int near *>(
			reinterpret_cast<char near *>(&p->pos[0].x) + page_offset
		);
		pos_y = reinterpret_cast<int near *>(
			reinterpret_cast<char near *>(&p->pos[0].y) + page_offset
		);
		*pos_x += p->velocity.x;
		*pos_y += p->velocity.y;
		top = (*pos_y >> 4);
		left = (*pos_x >> 4);
		if(
			(*pos_x <= (16 << 4)) ||
			(*pos_x >= (418 << 4)) ||
			(top > 384) ||
			(top < 8)
		) {
			p->flag = SF_REMOVE;
			continue;
		}
		top += scroll_line;
		if(top >= RES_Y) {
			top -= RES_Y;
		}
		if(!p->from_option) {
			shot_frames[i]++;
			if(p->age == 0) {
				if(skip != 2) {
					int patnum = p->patnum;
					if(patnum >= 50) {
						patnum += (shot_frames[i] & 1);
					}
					super_roll_put_tiny(left, top, patnum);
				}
			} else {
				if(p->age > 5) {
					p->flag = SF_REMOVE;
					continue;
				}
				if(skip != 2) {
					if(p->patnum < 124) {
						super_roll_put_tiny(left, top, (p->age + 74));
					} else {
						super_roll_put(left, top, SHOT_BIG_HIT_PATNUMS[p->age]);
					}
				}
				if((stage_frame & 3) == 0) {
					++p->age;
				}
			}
		} else {
			if(p->age > 1) {
				if((stage_frame % shot_option_frames_per_age) == 0) {
					++p->age;
					if(p->age > 4) {
						p->flag = SF_REMOVE;
						continue;
					}
				}
				if(stage_frame & 1) {
					p->velocity.y >>= 1;
				}
			} else if(p->patnum != 59) {
				p->velocity.y -= 4;
			}
			if(skip != 2) {
				super_roll_put_tiny(left, top, (p->patnum + p->age));
			}
		}
	}
}
