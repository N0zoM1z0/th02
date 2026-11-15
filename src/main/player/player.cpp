#pragma option -zCmain_01_TEXT -zPmain_01 -G

#include "th02/main/player/player.hpp"
#include "th02/main/player/shot.hpp"
#include "th02/main/hud/hud.hpp"
#include "th02/main/item/item.hpp"
#include "th02/main/main.hpp"
#include "th02/main/playperf.hpp"
#include "th02/main/score.hpp"
#include "th02/main/scroll.hpp"
#include "th02/main/spark.hpp"
#include "th02/snd/snd.h"
#include "th02/resident.hpp"
#include "libs/master.lib/pc98_gfx.hpp"
#include "th02/hardware/input.hpp"
#include "th02/hardware/pages.hpp"
#include "th02/sprites/main_pat.h"

// Coordinates
// -----------

static const screen_x_t PLAYER_LEFT_MIN = (PLAYFIELD_LEFT - 4);
static const screen_x_t PLAYER_LEFT_MAX = (PLAYFIELD_RIGHT - (PLAYER_W - 6));
static const screen_y_t PLAYER_TOP_MIN = PLAYFIELD_TOP;
static const screen_y_t PLAYER_TOP_MAX = (PLAYFIELD_BOTTOM - (PLAYER_H - 8));
// -----------

extern int player_patnum; // ACTUAL TYPE: main_patnum_t

// Function ordering fails
// -----------------------

// Uninitialized data, in the original order
uint8_t shot_cooldown_1;
uint8_t shot_cooldown_2;

uint8_t total_miss_count = 0;

extern const uint8_t POWER_RESET_FOR[SHOT_LEVEL_MAX + 1] = {
	1, 1, 4, 8, 16, 24, 32, 40, 52, 64
};

void near player_miss_update_and_render(void);
// -----------------------

#define negate_hits_if_invincible() { \
	if(player_invincible_via_bomb || ( \
		player_invincibility_time && !miss_active \
	)) { \
		player_is_hit = PLAYER_NOT_HIT; \
	} \
}

void pascal near player_move(pixel_t delta_x, pixel_t delta_y)
{
	player_topleft.y = *player_top_on_back_page;
	player_topleft.y += delta_y;
	if(player_topleft.y < PLAYER_TOP_MIN) {
		player_topleft.y = PLAYER_TOP_MIN;
	} else if(player_topleft.y > PLAYER_TOP_MAX) {
		player_topleft.y = PLAYER_TOP_MAX;
	}
	*player_top_on_back_page = player_topleft.y;

	// ZUN bloat: Could have been abstracted away if the comparison order and
	// operator were consistent...
	player_topleft.x = *player_left_on_back_page;
	player_topleft.x += delta_x;
	if(player_topleft.x >= PLAYER_LEFT_MAX) {
		player_topleft.x = PLAYER_LEFT_MAX;
	} else if(player_topleft.x <= PLAYER_LEFT_MIN) {
		player_topleft.x = PLAYER_LEFT_MIN;
	}
	*player_left_on_back_page = player_topleft.x;
}

void near player_update_and_render(void)
{
	// Update
	// ------

	negate_hits_if_invincible();
	if(player_invincibility_time > 0) {
		player_invincibility_time--;
	}

	shots_update_and_render();

	if(player_is_hit) {
		player_miss_update_and_render();
		if(player_is_hit == PLAYER_HIT_GAMEOVER) {
			quit = true;
			player_is_hit = PLAYER_NOT_HIT;
		}
		return;
	}
	// ------

	// Render
	// ------

	vram_y_t vram_top;
	if((player_invincibility_time & 3) < 3) {
		vram_top = scroll_screen_y_to_vram(vram_top, player_topleft.y);
		super_roll_put(player_topleft.x, vram_top, player_patnum);
	}
	if(power >= 8) {
		// ZUN bloat: Repeats the dereferencing every time it's used. (Yes, not
		// just the wordy name I chose for this variable!)
		#define option_left_left *player_option_left_left_on_back_page

		vram_top = scroll_screen_y_to_vram(
			vram_top, *player_option_left_top_on_back_page
		);
		super_roll_put_tiny(option_left_left, vram_top, player_option_patnum);
		super_roll_put_tiny(
			(option_left_left + PLAYER_OPTION_TO_OPTION_DISTANCE),
			vram_top,
			player_option_patnum
		);

		#undef option_left_left
	}
	// ------
}

inline void player_miss_sparks_add(int count) {
	// ZUN bug: Should subtract half the spark size to actually center them.
	sparks_add(
		(*player_left_on_back_page + (PLAYER_W / 2)),
		(*player_top_on_back_page  + (PLAYER_H / 2)),
		to_sp(8.0f),
		count,
		true
	);
}

void near player_miss_update_and_render(void)
{
	enum {
		KEYFRAME_PLAYCHAR_ANIM = 0,
		KEYFRAME_PLAYCHAR_ANIM_DONE = 24,
		KEYFRAME_RESPAWN = 43,
		KEYFRAME_RESPAWN_DONE = 68,

		PLAYCHAR_ANIM_FRAMES = (
			KEYFRAME_PLAYCHAR_ANIM_DONE - KEYFRAME_PLAYCHAR_ANIM
		),
	};

	if(miss_frame == 0) {
		snd_se_play(2);
		miss_active = true;
		stage_miss_count++;

		// ZUN bug: The fact that this function does not re-render the score
		// means that any existing [score_delta] will stop being animated. The
		// score display will therefore only refresh with the correct value
		// after the player gained another point.
		score_delta_commit();

		total_miss_count++;
		player_miss_sparks_add(16);
	}

	player_topleft.x = *player_left_on_back_page;
	player_topleft.y = *player_top_on_back_page;
	miss_frame++;
	if(miss_frame < KEYFRAME_PLAYCHAR_ANIM_DONE) {
		static_assert(PLAYCHAR_ANIM_FRAMES == (MISS_ANIM_CELS * 4));

		// ACTUAL TYPE: main_patnum_t
		int patnum = (PAT_PLAYCHAR_MISS + (miss_frame >> 2));

		vram_y_t vram_top = scroll_screen_y_to_vram(
			vram_top, *player_top_on_back_page
		);
		super_roll_put(*player_left_on_back_page, vram_top, patnum);
	} else if(miss_frame == KEYFRAME_PLAYCHAR_ANIM_DONE) {
		player_invincibility_time = MISS_INVINCIBILITY_FRAMES;
		if(playperf > 2) {
			playperf = 0;
		} else {
			playperf -= 2;
			if(playperf < playperf_min) {
				playperf = playperf_min;
			}
		}
		player_miss_sparks_add(32);
		if(lives == 0) {
			miss_frame = 0;
			items_miss_add_gameover = true;

			// ZUN quirk: Not centered
			items_miss_add(*player_left_on_back_page, *player_top_on_back_page);

			items_miss_add_gameover = false;
			player_is_hit = PLAYER_HIT_GAMEOVER;
		} else {
			extern const uint8_t POWER_RESET_FOR[SHOT_LEVEL_MAX + 1];
			power = POWER_RESET_FOR[shot_level];
			player_shot_level_update_and_hud_power_put();

			// ZUN quirk: Not centered
			items_miss_add(*player_left_on_back_page, *player_top_on_back_page);
		}
	} else if(miss_frame < KEYFRAME_RESPAWN) {
		// ZUN quirk: The only "death-induced hitbox shifting" that actually
		// exists in the code. (The infamous one during the Extra Stage midboss
		// is just players elevating happy collision detection accidents to a
		// "safespot" while disregarding the much more obvious effect of rank
		// on patterns; see
		//
		// 	https://rec98.nmlgc.net/blog/2025-02-24#myth-2025-02-24
		//
		// for documentation.
		//
		// This does have gameplay impact as the player's position continues to
		// be used for collision detection, and thus, potential entity removal.
		// Also, this can and will move the player past [PLAYER_TOP_MAX] if the
		// death occurred low enough.
		*player_top_on_back_page += 4;
	} else if(miss_frame == KEYFRAME_RESPAWN) {
		lives--;
		hud_lives_put();

		bombs = resident->start_bombs;
		if(lives == 0) {
			bombs += 2;
		}
		hud_bombs_put();

		// ZUN quirk: If the additions between [KEYFRAME_PLAYCHAR_ANIM_DONE]
		// and [KEYFRAME_RESPAWN] moved the player's Y position below [RES_Y],
		// the sparks spawned here will get wrapped back into the VRAM area and
		// seemingly strangely appear near the top of the playfield.
		// See sparks_add()'s implementation for why this actually is a quirk
		// and not a bug.
		player_miss_sparks_add(16);

		player_left_on_page[1] = player_left_on_page[0] = PLAYER_LEFT_START;
		player_top_on_page[1] = player_top_on_page[0] = (
			((PLAYFIELD_BOTTOM + PLAYFIELD_ROLL_MARGIN) - PLAYER_H) - 2
		);
	} else if(miss_frame < KEYFRAME_RESPAWN_DONE) {
		*player_top_on_back_page -= 2;
		vram_y_t vram_top = scroll_screen_y_to_vram(
			vram_top, *player_top_on_back_page
		);
		super_roll_put(*player_left_on_back_page, vram_top, PAT_PLAYCHAR_STILL);
	} else {
		miss_frame = 0;
		miss_active = false;
		player_is_hit = PLAYER_NOT_HIT;
	}
}

#pragma option -a2

extern int8_t playchar_speed_aligned_x;
extern int8_t playchar_speed_aligned_y;
extern int8_t playchar_speed_diagonal_x;
extern int8_t playchar_speed_diagonal_y;
extern void (near *playchar_shot_func)(void);
uint8_t shot_cycle_1 = 0xFF;
uint8_t shot_cycle_2 = 0xFF;
extern uint8_t shot_max_param_1;
extern uint8_t shot_max_param_2;
extern int8_t shot_max_spread;
extern uint8_t shot_param_1;
extern uint8_t shot_param_2;

extern "C" void pascal near player_bomb(void);

void near player_controls_update(void)
{
	register pixel_t delta_x;
	register pixel_t delta_y;

	negate_hits_if_invincible();
	if(player_is_hit) {
		return;
	}

	#define up_left() { \
		delta_y = -playchar_speed_diagonal_y; \
		delta_x = -playchar_speed_diagonal_x; \
		player_patnum = PAT_PLAYCHAR_LEFT; \
	}
	#define up_right() { \
		delta_y = -playchar_speed_diagonal_y; \
		delta_x = playchar_speed_diagonal_x; \
		player_patnum = PAT_PLAYCHAR_RIGHT; \
	}
	#define down_left() { \
		delta_y = playchar_speed_diagonal_y; \
		delta_x = -playchar_speed_diagonal_x; \
		player_patnum = PAT_PLAYCHAR_LEFT; \
	}
	#define down_right() { \
		delta_y = playchar_speed_diagonal_y; \
		delta_x = playchar_speed_diagonal_x; \
		player_patnum = PAT_PLAYCHAR_RIGHT; \
	}
	switch(key_det & INPUT_MOVEMENT_DIAGONAL) {
	case INPUT_UP_LEFT:   	goto up_left;
	case INPUT_UP_RIGHT:  	goto up_right;
	case INPUT_DOWN_LEFT: 	goto down_left;
	case INPUT_DOWN_RIGHT:	goto down_right;
	}
	switch(key_det & INPUT_MOVEMENT_ALIGNED) {
	case INPUT_UP:
		delta_y = -playchar_speed_aligned_y;
		delta_x = 0;
		player_patnum = PAT_PLAYCHAR_STILL;
		break;
	case INPUT_DOWN:
		delta_y = playchar_speed_aligned_y;
		delta_x = 0;
		player_patnum = PAT_PLAYCHAR_STILL;
		break;
	case INPUT_LEFT:
		delta_x = -playchar_speed_aligned_x;
		delta_y = 0;
		player_patnum = PAT_PLAYCHAR_LEFT;
		break;
	case INPUT_RIGHT:
		delta_x = playchar_speed_aligned_x;
		delta_y = 0;
		player_patnum = PAT_PLAYCHAR_RIGHT;
		break;
	case (INPUT_UP | INPUT_LEFT):   	up_left:   	up_left();   	break;
	case (INPUT_UP | INPUT_RIGHT):  	up_right:  	up_right();  	break;
	case (INPUT_DOWN | INPUT_LEFT): 	down_left: 	down_left(); 	break;
	case (INPUT_DOWN | INPUT_RIGHT):	down_right:	down_right();	break;
	default:
		delta_x = 0;
		delta_y = 0;
		player_patnum = PAT_PLAYCHAR_STILL;
		break;
	}
	#undef down_right
	#undef down_left
	#undef up_right
	#undef up_left

	if(playchar_speed_aligned_x == 5) {
		if(delta_x < 0) {
			delta_x += page_back;
		} else if(delta_x > 0) {
			delta_x -= page_back;
		}
		if(delta_y < 0) {
			delta_y += page_back;
		} else if(delta_y > 0) {
			delta_y -= page_back;
		}
	}
	player_move(delta_x, delta_y);
	*player_option_left_left_on_back_page = (
		(player_topleft.x - PLAYER_LEFT_TO_OPTION_LEFT_LEFT) - delta_x
	);
	*player_option_left_top_on_back_page = (
		(player_topleft.y + ((PLAYER_H / 2) - (PLAYER_OPTION_H / 2))) - delta_y
	);

	if(key_det & INPUT_SHOT) {
		if(shot_cycle_1 == 0xFF) {
			shot_cycle_1 = 0;
			shot_cooldown_1 = 0;
		} else if(shot_cycle_2 == 0xFF) {
			shot_cycle_2 = 0;
			shot_cooldown_2 = 0;
		}
	} else if(shot_cycle_1 >= 3) {
		shot_cycle_1 = 0xFF;
		shot_cycle_2 = 4;
	} else if(shot_cycle_2 >= 2) {
		shot_cycle_2 = 0xFF;
	}

	if(shot_cycle_1 != 0xFF) {
		if(shot_cooldown_1 == 0) {
			if(shot_level == SHOT_LEVEL_MAX) {
				shot_param_1 = shot_max_param_1;
				shot_param_2 = shot_max_param_2;
			} else {
				shot_param_1 = 0x34;
				shot_param_2 = 0x3F;
			}
			playchar_shot_func();
			snd_se_play(1);
			shot_cooldown_1 = 8;
			shot_cycle_1++;
			goto bomb;
		}
		shot_cooldown_1--;
	}
	if(shot_cycle_2 < 2) {
		if(shot_cooldown_2 == 0) {
			if(shot_level == SHOT_LEVEL_MAX) {
				shot_param_1 = shot_max_param_1;
				shot_param_2 = shot_max_param_2;
				if(key_det & (INPUT_DOWN | INPUT_DOWN_LEFT | INPUT_DOWN_RIGHT)) {
					shot_max_spread += 5;
					if(shot_max_spread > 26) {
						shot_max_spread = 26;
					}
				} else if(!(key_det & (INPUT_MOVEMENT_ALIGNED | INPUT_MOVEMENT_DIAGONAL))) {
					shot_max_spread += -5;
					if(shot_max_spread < 0) {
						shot_max_spread = 0;
					}
				}
			} else {
				shot_param_1 = 0x34;
				shot_param_2 = 0x3F;
			}
			playchar_shot_func();
			shot_cooldown_2 = 8;
			shot_cycle_2++;
		} else {
			snd_se_play(1);
			shot_cooldown_2--;
		}
	}
bomb:
	if(key_det & INPUT_BOMB) {
		player_bomb();
	}
}
