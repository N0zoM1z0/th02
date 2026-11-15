/* ReC98
 * -----
 * player_invalidate() of TH02's MAIN.EXE, compiled with -G
 */

#pragma option -zCmain_01_TEXT -zPmain_01 -G

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

void near shots_invalidate(void);

void near player_invalidate(void)
{
	int unused = 2;
	screen_x_t option_left;

	shots_invalidate();
	player_left_on_back_page = &player_left_on_page[page_back];
	player_top_on_back_page = &player_top_on_page[page_back];
	player_option_left_left_on_back_page = (
		&player_option_left_topleft[page_back].x
	);
	player_option_left_top_on_back_page = (
		&player_option_left_topleft[page_back].y
	);
	tiles_invalidate_rect(
		*player_left_on_back_page, *player_top_on_back_page, PLAYER_W, PLAYER_H
	);
	option_left = *player_option_left_left_on_back_page;
	tiles_invalidate_rect(
		option_left,
		*player_option_left_top_on_back_page,
		PLAYER_OPTION_W,
		PLAYER_OPTION_H
	);
	tiles_invalidate_rect(
		(option_left + PLAYER_OPTION_TO_OPTION_DISTANCE),
		*player_option_left_top_on_back_page,
		PLAYER_OPTION_W,
		PLAYER_OPTION_H
	);
	*player_left_on_back_page = player_left_on_page[page_front];
	*player_top_on_back_page = player_top_on_page[page_front];
}
