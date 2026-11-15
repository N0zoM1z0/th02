/* ReC98
 * -----
 * The game over and continue menu of TH02's MAIN.EXE
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

unsigned char gGAMEOVER[] = {
	gb_G, gb_A, gb_M, gb_E, gb_O, gb_V, gb_E, gb_R, gs_NULL
};
unsigned char gCONTINUE[] = {
	gb_C, gb_O, gb_N, gb_T, gb_I, gb_N, gb_U, gb_E, gs_QUESTION, gs_NULL
};
unsigned char gYES[] = { gb_Y, gb_E, gb_S, gs_NULL };
unsigned char gNO[4] = { gb_N, gb_O, gs_NULL };
unsigned char gCREDIT[] = { gb_C, gb_R, gb_E, gb_D, gb_I, gb_T, gs_NULL };

void pascal near continue_choice_put(int yes_atrb, int no_atrb)
{
	gaiji_putsa(24, 14, gYES, yes_atrb);
	gaiji_putsa(25, 15, gNO, no_atrb);
}

int near gameover_and_continue_menu(void)
{
	int input_state = 0;
	int i;
	int choice = 1;

	overlay_stage_leave_animate();
	PaletteTone = 50;
	palette_show();
	overlay_stage_enter_animate();
	for(i = 21; i >= 12; i--) {
		text_putsa(18, (i + 1), "                ", TX_WHITE);
		gaiji_putsa(18, i, gGAMEOVER, TX_WHITE);
		text_putsa(18, (23 - i), "                ", TX_WHITE);
		gaiji_putsa(18, (24 - i), gGAMEOVER, TX_WHITE);
		text_putsa((29 - i), 12, "                ", TX_WHITE);
		gaiji_putsa((30 - i), 12, gGAMEOVER, TX_WHITE);
		text_putsa((i + 7), 12, "                ", TX_WHITE);
		gaiji_putsa((i + 6), 12, gGAMEOVER, TX_WHITE);
		frame_delay(1);
	}
	frame_delay(30);
	key_det = 0;
	while(key_det == 0) {
		input_reset_sense();
	}
	overlay_wipe();
	regist_menu();
	overlay_wipe();
	if((resident->continues_used < 3) && (rank < RANK_EXTRA)) {
		gaiji_putsa(18, 12, gCONTINUE, TX_WHITE);
		continue_choice_put((TX_WHITE | TX_REVERSE), TX_WHITE);
		gaiji_putsa(18, 20, gCREDIT, TX_GREEN);
		i = (gb_3 - resident->continues_used);
		gaiji_putca(32, 20, i, TX_GREEN);
		choice = 1;
		while(1) {
			input_reset_sense();
			if((input_state == 0) && (key_det == 0)) {
				input_state = 1;
			} else if((input_state == 1) && (key_det != 0)) {
				input_state = 2;
			}
			if(key_det & INPUT_UP) {
				choice = 1;
				continue_choice_put((TX_WHITE | TX_REVERSE), TX_WHITE);
			}
			if(key_det & INPUT_DOWN) {
				choice = 0;
				continue_choice_put(TX_WHITE, (TX_WHITE | TX_REVERSE));
			}
			if((input_state == 2) && (
				(key_det & INPUT_SHOT) ||
				(key_det & INPUT_OK) ||
				(key_det & INPUT_CANCEL)
			)) {
				break;
			}
			frame_delay(1);
		}
		if(key_det & INPUT_CANCEL) {
			choice = 0;
		}
	} else {
		choice = 0;
	}
	resident->score_highest = (resident->score_highest < score)
		? score
		: resident->score_highest;
	lives = resident->start_lives;
	bombs = resident->start_bombs;
	power = POWER_MIN;
	resident->continues_used++;
	score_reset();
	item_bigpower_override = ((stage_id % 5) + 2);
	power_overflow = 10;
	PaletteTone = 100;
	palette_show();
	overlay_wipe();
	return choice;
}
