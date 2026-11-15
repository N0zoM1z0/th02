/* ReC98
 * -----
 * cfg_load() of TH02's MAIN.EXE. Its own TU, like in OP and the ending.
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

// Uninitialized data
resident_t far *resident;
int16_t resident_unused; // ZUN bloat
int playperf;

char rank = RANK_NORMAL;
char stage_id = 0;

int near cfg_load(void)
{
	resident_t __seg *resident_seg;

	file_ropen("huuma.cfg");
	file_seek(5, 0);
	file_read(&resident_seg, sizeof(resident_seg));
	file_close();
	if(resident_seg) {
		resident = resident_seg;
		stage_id = resident->stage;
		lives = resident->start_lives;
		bombs = resident->start_bombs;
		rank = resident->rank;
		power = resident->start_power;
		if(power == 0) {
			power++;
		}
		playperf = 0;
		item_bigpower_override = 0;
		return 1;
	}
	return 0;
}
