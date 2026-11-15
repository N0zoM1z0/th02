/* ReC98
 * -----
 * Code segment #5 of TH02's MAIN.EXE: loading and freeing the stage enemy
 * data (STAGE?.DT1)
 */

#include <mem.h>
#include "platform.h"
#include "pc98.h"
#include "libs/master.lib/master.hpp"
#include "libs/master.lib/pc98_gfx.hpp"
#include "th01/rank.h"
#include "th02/core/globals.hpp"
#include "th02/formats/map.hpp"
#include "th02/main/stage/stage.hpp"
#include "th02/main/enemy/enemy.hpp"

static const int STAGE_FORMATION_SCRIPT_SIZE = 64;
static const int STAGE_LINE_COLUMNS = 49;
extern int16_t far *stage_lines[STAGE_LINE_COLUMNS];
extern int stage_line_count;

// Uninitialized data, in the original order
int map_length;
uint8_t map_unused; // ZUN bloat
uint8_t stage_formation_script_count;
unsigned char stage_formation_scripts[40][STAGE_FORMATION_SCRIPT_SIZE];

void stage_data_load(void)
{
	int count;
	int j;
	void __seg *seg;
	int32_t size;
	char far *fn;
	stage_formation_t near *dest;
	unsigned char far *src;
	int i;

	fn = "STAGE .DT1";
	fn[5] = ('0' + stage_id);
	file_ropen(fn);
	file_read(&size, 4);
	seg = hmem_allocbyte(size);
	src = reinterpret_cast<unsigned char far *>(seg);
	file_read(src, size);
	file_close();

	map_length = *reinterpret_cast<int16_t far *>(src);
	src += 2;
	for(i = 0; i < map_length; i++) {
		map[i] = *src;
		src++;
	}

	count = *reinterpret_cast<int16_t far *>(src);
	src += 2;
	stage_formation_script_count = 0;
	for(dest = stage_formations, i = 0; i < count; i++, dest++) {
		memcpy(dest, src, 0x1C);
		src += 0x1C;
		dest->w = ((super_patsize[stage_formations[i].patnum] >> 8) << 3);
		dest->h = (super_patsize[stage_formations[i].patnum] & 0xFF);
		j = dest->speed;
		if(rank == RANK_EASY) {
			j <<= 1;
		// Unsigned in this translation unit: compared bytewise, unlike all
		// other rank checks in MAIN.EXE.
		} else if((static_cast<uint8_t>(rank) == RANK_HARD) || (static_cast<uint8_t>(rank) == RANK_LUNATIC)) {
			j >>= 1;
		}
		dest->speed = j;
		if(*src == 0xFE) {
			src++;
			dest->script = stage_formation_scripts[*src];
			src++;
			src++;
		} else {
			j = 0;
			do {
				stage_formation_scripts[stage_formation_script_count][j] = *src;
				src++;
				j++;
			} while(*src != 0xFF);
			src++;
			dest->script = stage_formation_scripts[stage_formation_script_count];
			stage_formation_script_count++;
		}
	}

	count = *reinterpret_cast<int16_t far *>(src);
	src += 2;
	stage_line_count = count;
	for(i = 0; i < STAGE_LINE_COLUMNS; i++) {
		stage_lines[i] = reinterpret_cast<int16_t far *>(
			hmem_allocbyte(count * sizeof(int16_t))
		);
	}
	for(i = 0; i < count; i++) {
		stage_lines[0][i] = *reinterpret_cast<int16_t far *>(src);
		src += 2;
		for(j = 1; j < STAGE_LINE_COLUMNS; j++) {
			stage_lines[j][i] = (*src == 0xFF) ? -1 : *src;
			src++;
		}
	}
	hmem_free(seg);
}

void stage_data_free(void)
{
	register int i;

	for(i = 0; i < STAGE_LINE_COLUMNS; i++) {
		if(stage_lines[i] != NULL) {
			hmem_free(reinterpret_cast<void __seg *>(FP_SEG(stage_lines[i])));
			stage_lines[i] = NULL;
		}
	}
}
