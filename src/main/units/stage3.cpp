/* ReC98
 * -----
 * Stage 3 of TH02's MAIN.EXE: background effect, midboss and the Five Magic
 * Stones
 */

#pragma option -zCmain_03_TEXT -zPmain_03 -G

#include "platform.h"
#include "x86real.h"
#include "pc98.h"
#include "libs/master.lib/master.hpp"
#include "libs/master.lib/pc98_gfx.hpp"
#include "th02/core/globals.hpp"
#include "th02/main/playfld.hpp"
#include "th02/main/scroll.hpp"
#include "th02/main/boss/b3.hpp"
#include "th02/main/boss/boss.hpp"
#include "th02/main/dialog/dialog.hpp"
#include "th02/main/hud/overlay.hpp"
#include "th02/main/stage/bonus.hpp"
#include "th02/main/stage/stage.hpp"
#include "th02/main/slowdown.hpp"
#include "th02/hardware/pages.hpp"
#include "th01/math/polar.hpp"
#include "th01/sprites/pellet.h"
#include "th02/sprites/main_pat.h"
#include "th02/math/randring.hpp"
#include "th02/snd/snd.h"
#include "th02/hardware/egc.hpp"
#include "th02/main/bullet/bullet.hpp"
#include "th02/main/player/player.hpp"
#include "th02/main/tile/tile.hpp"
#include "th01/rank.h"
#include "th02/v_colors.hpp"
#include "th02/main/score.hpp"
#include "th02/main/spark.hpp"
#include "th02/main/item/item.hpp"

static const pixel_t STONE_W = 32;
extern screen_x_t shot_target_left;
extern screen_y_t shot_target_top;

int pascal near shots_hittest(
	screen_x_t left, screen_y_t top, pixel_t w, pixel_t h
);

// Uninitialized data, in the original order
#pragma option -a2
int stone_patnum[STONE_COUNT];
uint8_t stones_phase;
uint8_t stones_pattern;
long stone_frames;
screen_x_t* midboss3_left_on_back_page[STONE_COUNT];
screen_y_t* midboss3_top_on_back_page[STONE_COUNT];
screen_x_t stone_left[STONE_COUNT];
screen_y_t stone_top[STONE_COUNT];
screen_x_t stones_bullet_left;
screen_y_t stones_bullet_top;
vram_y_t stones_y_22D9C;
pixel_t stage3_bg_radius;
screen_x_t stage3_bg_center_x;
screen_y_t stage3_bg_center_y;
unsigned char stage3_bg_angle;
screen_point_t stage3_bg_dots[PAGE_COUNT][64];
unsigned char midboss3_angle[MIDBOSS3_COUNT];
uint8_t midboss3_rounds;
int stones_pattern_frame;
screen_x_t stones_pillar_left;
uint8_t stones_pillar_state;
unsigned char angle_22FAF;
unsigned char angle_22FB0;
unsigned char angle_22FB1[STONE_COUNT];
uint8_t stones_ring_tile_flag[TILES_X];
uint8_t stones_ring_phase;
uint8_t stones_ring_i;
uint8_t stones_ring_done;
int stones_ring_tile_1;
int stones_ring_tile_2;
int stones_ring_tile_3;
#pragma option -a-

#include "th02/main/midboss/m3.cpp"
#include "th02/main/boss/b3.cpp"
