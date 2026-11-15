/* ReC98
 * -----
 * Process switching and shot type setup of TH02's MAIN.EXE
 */

#pragma option -zCmain_01_TEXT -zPmain_01

#include <stddef.h>
#include <process.h>
#include "platform.h"
#include "x86real.h"
#include "pc98.h"
#include "libs/master.lib/master.hpp"
#include "libs/master.lib/pc98_gfx.hpp"
#include "th02/resident.hpp"
#include "th01/rank.h"
#include "th02/core/globals.hpp"
#include "th02/gaiji/gaiji.h"
#include "th02/hardware/frmdelay.h"
#include "th02/hardware/input.hpp"
#include "th02/hardware/pages.hpp"
#include "th02/formats/mpn.hpp"
#include "th02/formats/pi.h"
#include "th02/hiscore/regist.h"
#include "th02/main/main.hpp"
#include "th02/main/playperf.hpp"
#include "th02/main/score.hpp"
#include "th02/main/hud/overlay.hpp"
#include "th02/main/item/item.hpp"
#include "th02/main/player/player.hpp"
#include "th02/main/player/shot.hpp"
#include "th02/main/stage/stage.hpp"
#include "th02/main/tile/tile.hpp"

void near sub_E24A(void);
void stage_data_free(void);
void near gaiji_free(void);
void game_exit(void);

int GameExecl(char *binary_fn)
{
	pi_free(0);
	sub_E24A();
	mpn_free();
	stage_data_free();
	super_free();
	graph_clear();
	text_clear();
	gaiji_free();
	game_exit();
	return execl(binary_fn, binary_fn, NULL);
}

extern uint8_t option_shots_active;
extern uint8_t byte_205DF;
extern uint8_t byte_205E0;
extern uint8_t stage_miss_count;
extern int shot_target_left;
extern int shot_target_top;
extern int shot_target_left_unused;
extern uint8_t byte_20611;
extern int8_t shot_option_frames_per_age;
extern int8_t playchar_speed_aligned_x;
extern int8_t playchar_speed_aligned_y;
extern int8_t playchar_speed_diagonal_x;
extern int8_t playchar_speed_diagonal_y;
extern void (near *playchar_shot_func)(void);
extern uint8_t shot_max_param_1;
extern uint8_t shot_max_param_2;
extern int8_t shot_max_spread;
void near shot_a(void);
void near shot_b(void);
void near shot_c(void);

void near sub_C5B0(void)
{
	int i;
	for(i = 0; i < SHOT_COUNT; i++) {
		shots[i].flag = SF_FREE;
	}
	option_shots_active = 0;
	player_invincibility_time = 0;
	player_invincible_via_bomb = false;
	quit = false;
	stage_miss_count = 0;
	miss_frame = 0;
	byte_205DF = 8;
	byte_205E0 = 0;
	player_is_hit = false;
	miss_active = false;
	player_option_patnum = PAT_OPTION_A;
	shot_target_left = -1;
	shot_target_left_unused = -1;
	shot_target_top = -1;
	shot_max_spread = 0;
	byte_20611 = 0;
	switch(resident->shottype) {
	case 0:
		playchar_shot_func = shot_a;
		playchar_speed_aligned_x = 5;
		playchar_speed_aligned_y = 5;
		playchar_speed_diagonal_x = 4;
		playchar_speed_diagonal_y = 4;
		shot_max_param_1 = 48;
		break;
	case 1:
		playchar_shot_func = shot_b;
		playchar_speed_aligned_x = 4;
		playchar_speed_aligned_y = 4;
		playchar_speed_diagonal_x = 3;
		playchar_speed_diagonal_y = 3;
		shot_max_param_1 = 52;
		shot_max_param_2 = 55;
		player_option_patnum++;
		break;
	case 2:
		playchar_shot_func = shot_c;
		playchar_speed_aligned_x = 3;
		playchar_speed_aligned_y = 3;
		playchar_speed_diagonal_x = 3;
		playchar_speed_diagonal_y = 3;
		shot_option_frames_per_age = 3;
		shot_max_param_1 = 125;
		shot_max_param_2 = 59;
		player_option_patnum += 2;
		break;
	}
}
