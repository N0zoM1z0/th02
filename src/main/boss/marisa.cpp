/// Stage 4 Boss - Marisa
/// ---------------------

#include "platform.h"
#include "pc98.h"
#include "libs/master.lib/master.hpp"
#include "libs/master.lib/pc98_gfx.hpp"
#include "th02/resident.hpp"
#include "th02/math/randring.hpp"
#include "th02/hardware/pages.hpp"
#include "th02/main/frames.hpp"
#include "th02/main/playfld.hpp"
#include "th02/main/boss/boss.hpp"
#include "th02/main/stage/bonus.hpp"
#include "th02/main/stage/stage.hpp"
#include "th02/main/hud/overlay.hpp"
#include "th02/main/dialog/dialog.hpp"
#include "th02/main/bullet/bullet.hpp"
#include "th02/main/player/player.hpp"
#include "th02/core/globals.hpp"
#include "th02/snd/snd.h"
#include "th02/sprites/main_pat.h"
#include "th02/sprites/bullet16.h"
#include "th01/math/subpixel.hpp"
#include "th01/rank.h"
#include "th01/math/polar.hpp"
#include "th02/main/score.hpp"
#include "th02/main/spark.hpp"
#include "th02/main/item/item.hpp"
#include "planar.h"
#include "decomp.hpp"
#include "th02/hardware/frmdelay.h"
#include "th02/main/tile/tile.hpp"
#include "th02/main/scroll.hpp"
#include "th02/v_colors.hpp"

void near dialog_pre(void);

// Marisa's four bits (orbs).
static const int BIT_COUNT = 4;
extern uint8_t byte_20671;
extern int16_t patnum_2064E;
extern uint8_t byte_2066E;
extern uint8_t byte_2066F[];
extern uint8_t angle_1E510;
extern uint8_t byte_1EB88;

// Like master.lib's PLANE_PUT, as expected by super_put_1plane().
static const int PLANE_PUT = (0xFF00 | GC_RMW);

int pascal near shots_hittest(
	screen_x_t left, screen_y_t top, pixel_t w, pixel_t h
);
void pascal near sub_FFF8(screen_x_t left, screen_y_t top, int frame);

extern int16_t shot_target_left;
extern int16_t shot_target_top;
extern uint8_t byte_1FFF8;
extern uint8_t byte_2066A;
extern bool byte_2066B;

void pascal sub_3E1C(screen_x_t left, screen_y_t top, unsigned char angle);
void sub_3F8A(void);
void sub_3EF4(void);

extern int16_t word_1FFF0;
extern int16_t word_1FFF2;
extern int16_t word_1FFF6;

void sub_13ABB(char far *fn);
void near dialog_post(void);
void near marisa_1B214(void);
void shots_clear(void);

extern vram_offset_t scroll_line_vo;

#define marisa_and_player_put() { \
	super_put_rect(marisa_topleft.x, marisa_topleft.y, patnum_2064E); \
	super_put_rect( \
		(marisa_topleft.x + 48), marisa_topleft.y, (patnum_2064E + 1) \
	); \
	player_and_options_put(); \
}

// Data, in the original order
// ---------------------------

uint8_t MARISA_BG_COLORS[6] = { 10, 12, 13, 13, 12, 10 };
uint8_t byte_1EE96 = 0;
int midboss4_kill_frame = 0;
int16_t marisa_defeat_frame = 0;
int16_t marisa_bit_flash_frames[BIT_COUNT] = { 0 };
uint8_t byte_1EEA4 = 2;
int8_t byte_1EEA5 = 1;

// Uninitialized data, in the original order
// -----------------------------------------

int16_t marisa_phase;
int16_t word_26CFE;
screen_y_t midboss4_bullet_top;
screen_x_t marisa_bit_left[PAGE_COUNT][BIT_COUNT];
screen_y_t marisa_bit_top[PAGE_COUNT][BIT_COUNT];
int16_t marisa_bit_hits[BIT_COUNT];
int16_t marisa_bit_state[BIT_COUNT];
int16_t marisa_bit_hit_this_frame[BIT_COUNT];
int16_t marisa_bit_radius[BIT_COUNT];
int16_t marisa_bit_speed[BIT_COUNT];
int16_t marisa_bit_state_sum;
int8_t marisa_bg_col_p;
int8_t marisa_pattern;
int8_t marisa_pattern_cycle;
uint8_t marisa_bit_patterns_seen;
uint8_t marisa_bit_angle[BIT_COUNT];
vram_y_t word_26D54;
screen_x_t far *marisa_bit_left_on_back_page[BIT_COUNT];
screen_y_t far *marisa_bit_top_on_back_page[BIT_COUNT];
point_t marisa_topleft;
bool16 midboss4_active;
uint8_t midboss4_pattern;
uint8_t midboss4_patterns_done;
uint8_t byte_26D7E;
uint8_t angle_26D7F;
uint8_t angle_26D80;
int8_t byte_26D81;
screen_x_t word_26D82;
screen_y_t word_26D84;
uint8_t angle_26D86;
uint8_t angle_26D87;
uint8_t angle_26D88;
uint8_t byte_26D89;
uint8_t marisa_bit_spin[BIT_COUNT];
int16_t word_26D8E;
int16_t word_26D90;
uint8_t marisa_pattern_rounds;

// Background
// ----------


void sub_19E2F(void)
{
	register screen_x_t left;
	register int image;
	int tile_image;
	vram_y_t top;

	if(page_back == 0) {
		egc_on();
		outport2(EGC_ACTIVEPLANEREG, 0xFFF7);
		outport2(EGC_READPLANEREG, 0x3FF);
		outport2(EGC_MASKREG, 0xFFFF);
		outport2(EGC_ADDRRESSREG, 0);
		outport2(EGC_BITLENGTHREG, 0xF);
		outport2(EGC_MODE_ROP_REG, (EGC_WS_PATREG + EGC_RL_MEMREAD));
		tiles_invalidate_rect(176, 156, 96, 96);
		tiles_egc_render();
		outport2(EGC_MODE_ROP_REG, (EGC_WS_ROP + 0xFC));
		word_26D54 = 152;
		word_26D54 += scroll_line;
		if(word_26D54 >= RES_Y) {
			word_26D54 -= RES_Y;
		}
		top = word_26D54;
		for(tile_image = 0; tile_image < 0x60; tile_image += 16) {
			for(
				left = 176, image = tile_image;
				left < 272;
				left += TILE_W, image++
			) {
				tile_egc_roll_copy_8(left, top, image);
			}
			top += TILE_H;
			if(top >= RES_Y) {
				top -= RES_Y;
			}
		}
		egc_off();
	}
}

// Midboss
// -------
extern int midboss_tile_row;
extern uint8_t pillar_frames_new;

void pascal near pillar_add(
	screen_x_t left, screen_y_t top, int duration, int patnum
);

#define midboss4_bullet_left() \
	((word_26CFE * 16) + marisa_topleft.x + 28)

int midboss4_invalidate(void)
{
	boss_left_on_back_page = &boss_left_on_page[page_back];
	boss_top_on_back_page = &boss_top_on_page[page_back];
	tiles_invalidate_rect(*boss_left_on_back_page, *boss_top_on_back_page, 64, 64);
	*boss_left_on_back_page = boss_left_on_page[page_front];
	*boss_top_on_back_page = boss_top_on_page[page_front];
	return midboss4_active;
}

void near midboss4_render(void)
{
	register vram_y_t vram_top = marisa_topleft.y;

	vram_top += scroll_line;
	if(vram_top < 0) {
		vram_top += RES_Y;
	} else if(vram_top >= RES_Y) {
		vram_top -= RES_Y;
	}
	if(byte_2066B) {
		snd_se_play(4);
		super_roll_put_1plane(
			marisa_topleft.x, vram_top, patnum_2064E, 0, super_plane(V_WHITE)
		);
		byte_2066B = false;
	} else {
		super_roll_put(marisa_topleft.x, vram_top, patnum_2064E);
	}
}

void near midboss4_kill_animate(void)
{
	register vram_y_t vram_top;
	register int zoom;

	if((boss_phase_frame % 24) == 0) {
		sparks_add(
			(marisa_topleft.x + 32), (marisa_topleft.y + 32), (8 << 4), 24, true
		);
	}
	zoom = 10;
	zoom += (midboss4_kill_frame >> 3);
	midboss4_kill_frame++;
	if(midboss4_kill_frame >= 64) {
		midboss4_kill_frame = 0;
		boss_phase_frame = 0;
		byte_2066A = 2;
		shot_target_left = -1;
		shot_target_top = -1;
		midboss4_active = false;
		word_26CFE = -1;
		midboss_tile_row = 0x660;
	} else {
		vram_top = marisa_topleft.y;
		vram_top += scroll_line;
		if(vram_top >= RES_Y) {
			vram_top -= RES_Y;
		}
		super_zoom(marisa_topleft.x, vram_top, zoom, 2);
	}
}

void near midboss4_hittest(void)
{
	register vram_y_t vram_top;
	register int damage;

	if((damage = shots_hittest(
		(marisa_topleft.x + 8), marisa_topleft.y, 48, 48
	)) != 0) {
		byte_2066B = true;
		boss_damage += damage;
		vram_top = marisa_topleft.y;
		vram_top += scroll_line;
		if(vram_top >= RES_Y) {
			vram_top -= RES_Y;
		}
		if((boss_damage >= 400) && (vram_top < 336) && (vram_top > 96)) {
			snd_se_play(2);
			byte_2066A = true;
			score_delta += 50000;
			if(midboss_tile_row == 0x660) {
				damage = IT_1UP;
			} else {
				damage = IT_BOMB;
			}
			items_add((marisa_topleft.x + 24), marisa_topleft.y, damage);
		}
	}
}

void near midboss4_pattern_0(void)
{
	if((boss_phase_frame % 16) == 0) {
		snd_se_play(3);
		bullets_add_pellet(
			midboss4_bullet_left(), (PLAYFIELD_TOP + 80), 0x40, BG_1, (5 << 4)
		);
	}
}

void near midboss4_pattern_1(void)
{
	uint8_t group;

	if((boss_phase_frame % 16) == 0) {
		snd_se_play(3);
		group = ((rank == RANK_EASY) ? BG_4_SPREAD_WIDE : BG_5_SPREAD_MEDIUM);
		bullets_add_pellet(
			midboss4_bullet_left(),
			(PLAYFIELD_TOP + 80),
			0x40,
			group,
			(boss_phase_frame - ((1 << 4) + 4))
		);
	}
}

void near midboss4_pattern_2(void)
{
	if((boss_phase_frame % 8) == 0) {
		bullets_add_16x16(
			midboss4_bullet_left(),
			(PLAYFIELD_TOP + 80),
			0x40,
			BSM_CHASE,
			PAT_BULLET16_OUTLINED_BALL_BEIGE,
			8
		);
	}
}

void near midboss4_pattern_3(void)
{
	if((boss_phase_frame % (16 - (rank * 2))) == 0) {
		pillar_frames_new = 16;
		pillar_add(midboss4_bullet_left(), 80, 6, 0x6F);
	}
}

void near midboss4_pattern_4(void)
{
	register int i;

	if((boss_phase_frame % 16) == 0) {
		snd_se_play(7);
		for(i = 0; i < 4; i++) {
			bullets_add_pellet(
				midboss4_bullet_left(),
				midboss4_bullet_top,
				(randring2_next8_and(0x3F) + 0x20),
				BG_1,
				((4 << 4) + 6)
			);
		}
	}
}

#define midboss4_turn_around() { \
	boss_phase_frame = 2; /* Skip the fly-in animation */ \
	patnum_2064E = 148; \
	word_26CFE *= -1; \
	midboss4_pattern = (randring2_next8() % 5); \
}

void midboss4_update_and_render(void)
{
	shot_target_left = (marisa_topleft.x + 24);
	shot_target_top = (marisa_topleft.y + 24);
	boss_phase_frame++;
	if(boss_phase_frame == 1) {
		if(word_26CFE == 0) {
			boss_left_on_page[0] = (PLAYFIELD_LEFT + 32);
			patnum_2064E = 150;
		} else {
			boss_left_on_page[0] = (PLAYFIELD_RIGHT - 32 - 64);
			patnum_2064E = 149;
		}
		boss_left_on_page[1] = boss_left_on_page[0];
		boss_top_on_page[0] = (PLAYFIELD_TOP - 32);
		boss_top_on_page[1] = (PLAYFIELD_TOP - 32);
		boss_left_on_back_page = &boss_left_on_page[0];
		boss_top_on_back_page = &boss_top_on_page[0];
		boss_damage = 0;
		byte_2066A = false;
		byte_2066B = false;
		marisa_phase = 0;
		word_26CFE = ((word_26CFE == 0) ? 1 : -1);
		midboss4_patterns_done = 0;
		midboss4_pattern = 0;
		midboss4_active = true;
	}
	midboss4_bullet_top = (*boss_top_on_back_page + 48);
	marisa_topleft.y = *boss_top_on_back_page;
	if(!byte_2066A) {
		if(marisa_phase == 0) {
			*boss_left_on_back_page += (word_26CFE * 8);
			*boss_top_on_back_page += 2;
			marisa_topleft.x = *boss_left_on_back_page;
			marisa_topleft.y = *boss_top_on_back_page;
			if((boss_phase_frame & 7) == 0) {
				snd_se_play(7);
				bullets_add_pellet(
					midboss4_bullet_left(),
					midboss4_bullet_top,
					0x40,
					BG_1,
					(5 << 4)
				);
			}
			if(boss_phase_frame >= 32) {
				marisa_phase = 1;
				midboss4_turn_around();
			}
		} else if((marisa_phase == 1) && (boss_phase_frame > 50)) {
			if(word_26CFE == -1) {
				patnum_2064E = 149;
			} else {
				patnum_2064E = 150;
			}
			*boss_left_on_back_page += (word_26CFE * 4);
			marisa_topleft.x = *boss_left_on_back_page;
			switch(midboss4_pattern) {
			case 0: midboss4_pattern_0(); break;
			case 1: midboss4_pattern_1(); break;
			case 2: midboss4_pattern_2(); break;
			case 3: midboss4_pattern_3(); break;
			case 4: midboss4_pattern_4(); break;
			}
			if(midboss4_patterns_done < 12) {
				if(boss_phase_frame >= 114) {
					midboss4_turn_around();
					midboss4_patterns_done++;
				}
			} else if((marisa_topleft.x <= 0) || (marisa_topleft.x >= 384)) {
				boss_phase_frame = 0;
				word_26CFE = -1;
				midboss_tile_row = 0x660;
				midboss4_active = false;
				return;
			}
		}
	}
	if(byte_2066A) {
		midboss4_kill_animate();
		return;
	}
	midboss4_hittest();
	midboss4_render();
}

void pascal near sub_1A423(screen_x_t left, screen_y_t top)
{
	register int col;
	register int patnum;
	int row;

	patnum = 0;
	for(row = 0; row < 8; row++) {
		for(col = 0; col < 8; col++, patnum++) {
			super_put((left + (col * 32)), (top + (row * 32)), (128 + patnum));
		}
	}
}

void near sub_1A46B(void)
{
	register int i;
	register vram_y_t top_1 = 0;
	vram_y_t top_2 = 0;

	top_1 = 320;
	top_1 += scroll_line;
	if(top_1 >= RES_Y) {
		top_1 -= RES_Y;
	}
	top_2 += (top_1 + 32);
	if(top_2 >= RES_Y) {
		top_2 -= RES_Y;
	}
	super_roll_put(32, top_1, 34);
	super_roll_put(32, top_2, 42);
	super_roll_put(64, top_1, 35);
	super_roll_put(64, top_2, 43);
	super_roll_put(96, top_1, 36);
	super_roll_put(96, top_2, 44);
	for(i = 0; i < 8; i++) {
		super_roll_put(((i * 32) + 128), top_1, 37);
		super_roll_put(((i * 32) + 128), top_2, 45);
	}
	super_roll_put(384, top_1, 38);
	super_roll_put(384, top_2, 46);
}

void near sub_1A529(void)
{
	register int i;

	grc_setclip(PLAYFIELD_LEFT, PLAYFIELD_TOP, PLAYFIELD_RIGHT, 320);
	snd_se_play(9);
	for(i = 150; i > 0; i--) {
		grcg_setcolor(GC_RMW, 10);
		grcg_circlefill(224, 144, i);
		grcg_setcolor(GC_RMW, 13);
		grcg_circle(224, 144, ((i % 50) * 4));
		grcg_setcolor(GC_RMW, V_WHITE);
		grcg_circle(224, 144, ((i % 30) * 7));
		frame_delay(1);
		grcg_setcolor(GC_RMW, 0);
		grcg_circlefill(224, 144, i);
		grcg_circle(224, 144, ((i % 50) * 4));
		grcg_circle(224, 144, ((i % 30) * 7));
		PaletteTone = ((((150 - i) / 3) * 2) + 100);
		palette_show();
		snd_se_update();
	}
	grcg_setcolor(GC_RMW, 0);
	grcg_fill();
	grcg_off();
	sub_1A423(96, 0);
	player_and_options_put();
	sub_1A46B();
	for(i = 0; i < 10; i++) {
		PaletteTone = (200 - (i * 10));
		palette_show();
		frame_delay(3);
	}
	PaletteTone = 100;
	palette_show();
}

void near sub_1A6C5(void)
{
	player_top_on_page[page_front] = player_topleft.y;
	scroll_line = 0;
	scroll_line_vo = 0;
	graph_accesspage(0);
	graph_clear();
	graph_accesspage(1);
	graph_clear();
	vsync_Count1 = 0;
	frame_delay(1);
	graph_scrollup(0);
	graph_accesspage(page_front);
	super_roll_put(player_topleft.x, player_topleft.y, PAT_PLAYCHAR_STILL);
	player_option_left_topleft[page_back].x = (
		player_topleft.x - PLAYER_LEFT_TO_OPTION_LEFT_LEFT
	);
	player_option_left_topleft[page_back].y = (
		player_topleft.y + ((PLAYER_H / 2) - (PLAYER_OPTION_H / 2))
	);
	player_option_left_topleft[page_front].x = (
		player_option_left_topleft[page_back].x
	);
	player_option_left_topleft[page_front].y = (
		player_option_left_topleft[page_back].y
	);
	super_roll_put_tiny(
		player_option_left_topleft[page_back].x,
		player_option_left_topleft[page_back].y,
		PAT_OPTION_A
	);
	super_roll_put_tiny(
		(player_option_left_topleft[page_back].x +
		PLAYER_OPTION_TO_OPTION_DISTANCE),
		player_option_left_topleft[page_back].y,
		PAT_OPTION_A
	);
}

void marisa_init(void)
{
	register int i;

	sub_1A6C5();
	dialog_pre();
	super_clean(128, 192);
	super_patnum = 128;
	super_entry_bfnt("mima.bft");
	dialog_script_stage4_pre_intro_animate();
	vsync_Count1 = 0;
	frame_delay(10);
	sub_1A529();
	dialog_script_generic_part_animate(DS_PREBOSS);
	super_clean(128, 511);
	super_patnum = 128;
	super_entry_bfnt("stage3_b.bft");
	super_entry_bfnt("stage3_b.btt");
	boss_left_on_page[0] = (PLAYFIELD_LEFT + (PLAYFIELD_W / 2) - 48);
	boss_left_on_page[1] = boss_left_on_page[0];
	boss_top_on_page[0] = (PLAYFIELD_TOP + 48);
	boss_top_on_page[1] = boss_top_on_page[0];
	marisa_topleft.x = (PLAYFIELD_LEFT + (PLAYFIELD_W / 2) - 48);
	marisa_topleft.y = (PLAYFIELD_TOP + 48);
	patnum_2064E = 128;
	palette_white_out(1);
	grcg_setcolor(GC_RMW, 0);
	grcg_fill();
	grcg_off();
	grc_setclip(PLAYFIELD_LEFT, 0, PLAYFIELD_RIGHT, (RES_Y - 1));
	marisa_and_player_put();
	snd_se_play(10);
	snd_se_update();
	sub_1A46B();
	palette_white_in(1);
	sub_13ABB("boss3.m");
	dialog_script_stage4_pre_marisa_animate();
	graph_clear();
	marisa_and_player_put();
	graph_copy_page(page_front);
	graph_accesspage(page_back);
	graph_showpage(page_front);
	tile_mode = TM_NONE;
	shots_clear();
	boss_damage = 0;
	byte_2066A = false;
	boss_phase_frame = 0;
	byte_2066B = false;
	marisa_phase = 0;
	byte_1EE96 = 0;
	for(i = 0; i < BIT_COUNT; i++) {
		marisa_bit_hits[i] = 0;
		marisa_bit_state[i] = 2;
		marisa_bit_hit_this_frame[i] = 0;
	}
	byte_26D7E = 1;
	byte_1FFF8 = 9;
	word_1FFF6 = 24;
	word_1FFF0 = 24;
	word_1FFF2 = 4;
	word_26CFE = 0;
	marisa_bg_col_p = 0;
	marisa_pattern = 0;
	marisa_pattern_cycle = 0;
	marisa_bit_patterns_seen = 0;
	angle_1E510 = -0x20;
	marisa_1B214();
}

void near marisa_1AA60(void)
{
	int damage;

	if(byte_2066A) {
		return;
	}
	if((damage = shots_hittest(
		(marisa_topleft.x + 24), (marisa_topleft.y + 24), 48, 48
	)) != 0) {
		if(marisa_bit_state_sum >= 8) {
			snd_se_play(4);
			byte_2066B = true;
			boss_damage += (byte_1EE96 * damage);
		} else {
			snd_se_play(11);
		}
		if(boss_damage >= 900) {
			byte_2066A = true;
			score_delta += 20000;
			player_invincibility_time = BOSS_DEFEAT_INVINCIBILITY_FRAMES;
		}
	}
	if(
		(player_left_on_page[page_front] > (marisa_topleft.x - 16)) &&
		(player_left_on_page[page_front] < (marisa_topleft.x + 48)) &&
		(player_top_on_page[page_front] > (marisa_topleft.y - 16)) &&
		(player_top_on_page[page_front] < (marisa_topleft.y + 48))
	) {
		player_is_hit = PLAYER_HIT;
	}
}

void near marisa_1AB35(void)
{
	register int i;

	byte_1EB88 = 3;
	for(i = 0; i < BIT_COUNT; i++) {
		if(marisa_bit_state[i] != 0) {
			continue;
		}
		if(shots_hittest(
			(*marisa_bit_left_on_back_page[i] + 4),
			*marisa_bit_top_on_back_page[i],
			24,
			32
		)) {
			marisa_bit_hit_this_frame[i] = 1;
			marisa_bit_hits[i]++;
			if(marisa_bit_hits[i] >= 110) {
				if((randring2_next8() % 80) >= power) {
					items_add(
						(*marisa_bit_left_on_back_page[i] + 8),
						(*marisa_bit_top_on_back_page[i] + 8),
						IT_BIGPOWER
					);
				}
				snd_se_play(2);
				marisa_bit_state[i] = 1;
				score_delta += 5000;
			}
		}
		if(
			(player_left_on_page[page_front] > (*marisa_bit_left_on_back_page[i] - 16)) &&
			(player_left_on_page[page_front] < (*marisa_bit_left_on_back_page[i] + 16)) &&
			(player_top_on_page[page_front] > (*marisa_bit_top_on_back_page[i] - 16)) &&
			(player_top_on_page[page_front] < (*marisa_bit_top_on_back_page[i] + 16))
		) {
			player_is_hit = PLAYER_HIT;
		}
	}
	byte_1EB88 = 1;
}

int near marisa_1AC7B(void)
{
	register int zoom = 10;

	sub_FFF8(
		(marisa_topleft.x + 40), (marisa_topleft.y + 40), marisa_defeat_frame
	);
	if(marisa_defeat_frame >= 24) {
		if(marisa_defeat_frame == 56) {
			angle_1E510 = 0x20;
		}
		sub_FFF8(
			(marisa_topleft.x + 40),
			(marisa_topleft.y + 40),
			(marisa_defeat_frame - 24)
		);
		if(marisa_defeat_frame >= 48) {
			if(marisa_defeat_frame == 80) {
				angle_1E510 = 0x00;
			}
			sub_FFF8(
				(marisa_topleft.x + 40),
				(marisa_topleft.y + 40),
				(marisa_defeat_frame - 48)
			);
			if(
				(marisa_defeat_frame >= 64) &&
				((marisa_defeat_frame & 0x0F) == 0)
			) {
				sparks_add(
					(marisa_topleft.x + 48),
					(marisa_topleft.y + 48),
					to_sp(8.5f),
					24,
					true
				);
			}
		}
	}
	zoom += ((marisa_defeat_frame - 32) >> 4);
	marisa_defeat_frame++;
	if(marisa_defeat_frame < 32) {
		super_put_rect(marisa_topleft.x, marisa_topleft.y, patnum_2064E);
		super_put_rect(
			(marisa_topleft.x + 48), marisa_topleft.y, (patnum_2064E + 1)
		);
	} else {
		super_zoom(marisa_topleft.x, marisa_topleft.y, zoom, 3);
	}
	if(marisa_defeat_frame >= 100) {
		marisa_defeat_frame = 0;
		byte_2066A = 3;
		return true;
	}
	return false;
}

int near marisa_1AD80(int bit)
{
	vram_x_t vram_left;
	int patnum;

	sparks_add(
		(*marisa_bit_left_on_back_page[bit] + 16),
		(*marisa_bit_top_on_back_page[bit] + 16),
		to_sp(3.75f),
		2,
		false
	);
	patnum = 10;
	patnum += (marisa_bit_flash_frames[bit] / 6);
	marisa_bit_flash_frames[bit]++;
	if(marisa_bit_flash_frames[bit] >= 0x30) {
		marisa_bit_flash_frames[bit] = 0;
		marisa_bit_state[bit] = 2;
		graph_accesspage(page_front);
		grcg_setcolor(GC_RMW, 0);
		vram_left = (marisa_bit_left[page_front][bit] >> 3);
		grcg_byteboxfill_x(
			vram_left,
			marisa_bit_top[page_front][bit],
			(vram_left + 5),
			(marisa_bit_top[page_front][bit] + 32)
		);
		grcg_off();
		graph_accesspage(page_back);
		marisa_bit_hits[bit] = 0;
		return 1;
	}
	super_put_rect(
		*marisa_bit_left_on_back_page[bit],
		*marisa_bit_top_on_back_page[bit],
		patnum
	);
	return 0;
}

#define marisa_flash_put_part(part, x_offset) { \
	for(row = 0, y = marisa_topleft.y; row < 96; row++, vo += ROW_SIZE, y++) { \
		if(y >= 0x180) { \
			break; \
		} \
		for( \
			col = 0, x = (marisa_topleft.x + x_offset); \
			col < 6; \
			col++, x += 8, sprite++ \
		) { \
			if((x > 0) && (x < 0x1A0) && (y >= 16)) { \
				dots = ((*sprite >> shift) + (*sprite << shift_r)); \
				*reinterpret_cast<dots16_t far *>( \
					(VRAM_PLANE_B + vo) + col \
				) = dots; \
			} \
		} \
	} \
}

void near marisa_1AE98(void)
{
	int row;
	vram_offset_t vo;
	vram_offset_t vo_topleft;
	screen_y_t y;
	unsigned char shift;
	uint8_t far *sprite;
	unsigned char shift_r;
	dots16_t dots;
	register int col;
	register screen_x_t x;

	if(byte_2066B) {
		shift = (marisa_topleft.x & 7);
		shift_r = (16 - shift);
		byte_2066B = false;
		vo = vo_topleft = (
			(marisa_topleft.x >> 3) +
			(marisa_topleft.y << 6) +
			(marisa_topleft.y << 4)
		);
		grcg_setcolor(GC_RMW, 4);
		sprite = reinterpret_cast<uint8_t far *>(
			MK_FP(super_patdata[patnum_2064E], 0)
		);
		marisa_flash_put_part(0, 0);
		sprite = reinterpret_cast<uint8_t far *>(
			MK_FP(super_patdata[patnum_2064E + 1], 0)
		);
		vo = (vo_topleft + 6);
		marisa_flash_put_part(1, 48);
		grcg_off();
	} else {
		super_put_rect(marisa_topleft.x, marisa_topleft.y, patnum_2064E);
		super_put_rect(
			(marisa_topleft.x + 48), marisa_topleft.y, (patnum_2064E + 1)
		);
	}
}

void near marisa_1B025(void)
{
	int alive[BIT_COUNT + 1];
	int bit_a;
	int bit_b;
	register int i;
	register int alive_count;

	grcg_setcolor(GC_RMW, 3);
	for(i = 0, alive_count = 0; i < BIT_COUNT; i++) {
		if(marisa_bit_state[i] == 0) {
			alive[alive_count] = i;
			alive_count++;
		}
	}
	alive[alive_count] = alive[0];
	for(i = 0; i < alive_count; i++) {
		bit_a = alive[i];
		bit_b = alive[i + 1];
		grcg_line(
			(*marisa_bit_left_on_back_page[bit_a] + 16),
			(*marisa_bit_top_on_back_page[bit_a] + 16),
			(*marisa_bit_left_on_back_page[bit_b] + 16),
			(*marisa_bit_top_on_back_page[bit_b] + 16)
		);
	}
	grcg_off();
	for(i = 0; i < BIT_COUNT; i++) {
		if(marisa_bit_state[i] < 1) {
			if(marisa_bit_hit_this_frame[i] != 0) {
				if(
					(*marisa_bit_left_on_back_page[i] > 0) &&
					(*marisa_bit_left_on_back_page[i] < 0x1A0) &&
					(*marisa_bit_top_on_back_page[i] >= 0) &&
					(*marisa_bit_top_on_back_page[i] <= 0x170)
				) {
					super_put_1plane(
						*marisa_bit_left_on_back_page[i],
						*marisa_bit_top_on_back_page[i],
						(i + 138),
						GC_BRGI,
						PLANE_PUT
					);
				}
				marisa_bit_hit_this_frame[i] = 0;
			} else {
				super_put_rect(
					*marisa_bit_left_on_back_page[i],
					*marisa_bit_top_on_back_page[i],
					(i + 138)
				);
			}
		}
	}
}

void near marisa_1B19D(void)
{
	if(boss_phase_frame == 50) {
		snd_se_play(9);
		patnum_2064E = 130;
	} else if(boss_phase_frame == 100) {
		snd_se_play(10);
		patnum_2064E = 132;
	} else if(boss_phase_frame == 108) {
		snd_se_play(10);
		patnum_2064E = 134;
	} else if(boss_phase_frame == 116) {
		snd_se_play(10);
		patnum_2064E = 136;
	} else if(boss_phase_frame == 130) {
		patnum_2064E = 128;
	}
}

void near marisa_1B214(void)
{
	if(rank != RANK_EASY) {
		byte_2066E = BG_5_SPREAD_MEDIUM_AIMED;
		byte_2066F[0] = BG_4_SPREAD_MEDIUM_AIMED;
		byte_2066F[1] = BG_2_SPREAD_MEDIUM;
		byte_20671 = 4;
	} else {
		byte_2066E = BG_5_SPREAD_WIDE_AIMED;
		byte_2066F[0] = BG_4_SPREAD_WIDE_AIMED;
		byte_2066F[1] = BG_1;
		byte_20671 = 2;
	}
}

void near marisa_1B24A(void)
{
	if(boss_phase_frame < 50) {
		return;
	}
	marisa_1B19D();
	if(boss_phase_frame == 50) {
		angle_26D7F = 0x18;
	} else if(boss_phase_frame == 130) {
		boss_phase_frame = 0;
		byte_26D7E = 0;
	}
	if((boss_phase_frame > 100) && ((boss_phase_frame % 3) == 0)) {
		if(byte_26D7E != 0) {
			bullets_add_pellet(
				(marisa_topleft.x + 36),
				(marisa_topleft.y + 64),
				angle_26D7F,
				BG_1,
				to_sp(3.75f)
			);
			bullets_add_pellet(
				(marisa_topleft.x + 52),
				(marisa_topleft.y + 64),
				angle_26D7F,
				BG_1,
				to_sp(3.75f)
			);
		} else {
			bullets_add_pellet(
				(marisa_topleft.x + 44),
				(marisa_topleft.y + 64),
				angle_26D7F,
				BG_1,
				to_sp(3.125f)
			);
		}
		angle_26D7F += 0x0A;
	}
}

void near marisa_1B2E9(void)
{
	if(boss_phase_frame < 50) {
		return;
	}
	if(boss_phase_frame == 50) {
		snd_se_play(9);
		patnum_2064E = 130;
	}
	if((boss_phase_frame % 24) == 0) {
		bullets_add_pellet(
			(marisa_topleft.x + 64),
			(marisa_topleft.y + 16),
			0x00,
			byte_2066E,
			to_sp(3.375f)
		);
		bullets_add_pellet(
			(marisa_topleft.x + 64),
			(marisa_topleft.y + 16),
			0x00,
			byte_2066F[0],
			to_sp(3.75f)
		);
	}
	if(boss_phase_frame > 192) {
		boss_phase_frame = 0;
		patnum_2064E = 128;
	}
}

void near marisa_1B35F(void)
{
	if(boss_phase_frame < 50) {
		return;
	}
	marisa_1B19D();
	if(boss_phase_frame == 50) {
		angle_26D80 = randring2_next8();
		bullet_special.u1.drift_angle = byte_1EEA4;
		bullet_special.u2.drift_speed.v = 2;
		bullet_special.u3.drift_frames = 128;
		byte_1EEA4 *= -1;
	} else if(boss_phase_frame == 130) {
		boss_phase_frame = 0;
	}
	if((boss_phase_frame >= 100) && ((boss_phase_frame & 3) == 0)) {
		bullets_add_16x16(
			(marisa_topleft.x + 44),
			(marisa_topleft.y + 64),
			angle_26D80,
			BSM_DRIFT_ANGLE_AND_SPEED,
			PAT_BULLET16_STAR,
			to_sp(2.5f)
		);
		angle_26D80 += 0x2B;
	}
}

void near marisa_1B3DE(void)
{
	register int i;

	for(i = 0; i < BIT_COUNT; i++) {
		if(marisa_bit_state[i] == 0) {
			marisa_bit_angle[i] += marisa_bit_speed[i];
			*marisa_bit_left_on_back_page[i] = (polar_x_fast(
				marisa_topleft.x, marisa_bit_radius[i], marisa_bit_angle[i]
			) + 32);
			*marisa_bit_top_on_back_page[i] = (polar_y_fast(
				marisa_topleft.y, marisa_bit_radius[i], marisa_bit_angle[i]
			) + 32);
		}
	}
}

void near marisa_1B477(void)
{
	int i;
	register screen_x_t left;
	register screen_y_t top;

	if(boss_phase_frame < 100) {
		return;
	}
	if(boss_phase_frame == 100) {
		snd_se_play(9);
		patnum_2064E = 130;
	} else if(boss_phase_frame == 130) {
		left = (marisa_topleft.x + 32);
		top = (marisa_topleft.y + 32);
		for(i = 0; i < BIT_COUNT; i++) {
			marisa_bit_state[i] = 0;
			marisa_bit_angle[i] = (i << 6);
			marisa_bit_radius[i] = 8;
			marisa_bit_speed[i] = 2;
			marisa_bit_left[0][i] = left;
			marisa_bit_left[1][i] = left;
			marisa_bit_top[0][i] = top;
			marisa_bit_top[1][i] = top;
		}
	} else if(boss_phase_frame == 154) {
		patnum_2064E = 128;
		boss_phase_frame = 0;
	}
	if(boss_phase_frame > 130) {
		for(i = 0; i < BIT_COUNT; i++) {
			marisa_bit_radius[i] += 2;
		}
	}
}

void near marisa_1B555(void)
{
	register int bit;

	if(boss_phase_frame < 10) {
		return;
	}
	if(boss_phase_frame == 10) {
		byte_26D81 = ((marisa_topleft.x + 32) < player_topleft.x) ? 1 : -1;
		word_26D82 = marisa_topleft.x;
		word_26D84 = (marisa_topleft.y + 112);
		angle_26D86 = -0x40;
	}
	angle_26D86 += byte_26D81;
	*boss_left_on_back_page = polar_x_fast(word_26D82, 0x70, angle_26D86);
	*boss_top_on_back_page = polar_y_fast(word_26D84, 0x70, angle_26D86);
	marisa_topleft.x = *boss_left_on_back_page;
	marisa_topleft.y = *boss_top_on_back_page;
	if((boss_phase_frame & 0x0F) == 0) {
		bit = randring2_next8_and(BIT_COUNT - 1);
		if(marisa_bit_state[bit] == 0) {
			bullets_add_pellet(
				(*marisa_bit_left_on_back_page[bit] + 12),
				(*marisa_bit_top_on_back_page[bit] + 12),
				0x00,
				BG_1_AIMED,
				to_sp(5.0f)
			);
		}
		if((marisa_bit_state_sum == 8) && (bit < 1)) {
			bullets_add_pellet(
				(marisa_topleft.x + 64),
				(marisa_topleft.y + 16),
				0x00,
				BG_3_SPREAD_NARROW_AIMED,
				to_sp(5.0f)
			);
		}
	}
	if(boss_phase_frame >= 266) {
		boss_phase_frame = 0;
	}
}

void pascal near marisa_1B665(int frame)
{
	if(boss_phase_frame < frame) {
		return;
	}
	if(boss_phase_frame == frame) {
		snd_se_play(9);
		patnum_2064E = 130;
	} else if((frame + 30) == boss_phase_frame) {
		patnum_2064E = 130;
	} else if((frame + 36) == boss_phase_frame) {
		patnum_2064E = 132;
	} else if((frame + 42) == boss_phase_frame) {
		patnum_2064E = 134;
	} else if((frame + 50) == boss_phase_frame) {
		patnum_2064E = 136;
	} else if((frame + 60) == boss_phase_frame) {
		patnum_2064E = 128;
	}
}

#define marisa_bit_speed_set(v0_and_2, v1_and_3) { \
	marisa_bit_speed[0] = v0_and_2; \
	marisa_bit_speed[2] = v0_and_2; \
	marisa_bit_speed[1] = v1_and_3; \
	marisa_bit_speed[3] = v1_and_3; \
}

void near marisa_1B6DA(void)
{
	register int i;

	if(boss_phase_frame < 50) {
		return;
	}
	if(boss_phase_frame < 280) {
		for(i = 0; i < BIT_COUNT; i++) {
			marisa_bit_radius[i]++;
		}
	} else if(boss_phase_frame < 510) {
		for(i = 0; i < BIT_COUNT; i++) {
			marisa_bit_radius[i]--;
		}
	} else {
		boss_phase_frame = 0;
		patnum_2064E = 128;
	}
	marisa_1B665(220);
	if(boss_phase_frame == 220) {
		angle_26D87 = 0;
	}
	if(marisa_bit_state_sum == 8) {
		patnum_2064E = 130;
	}
	if(
		((boss_phase_frame >= 250) && (boss_phase_frame < 270)) ||
		(marisa_bit_state_sum == 8)
	) {
		if(marisa_bit_state_sum != 8) {
			angle_26D87 += 0x0D;
			if(rank >= (boss_phase_frame & 1)) {
				bullets_add_pellet(
					(marisa_topleft.x + 44),
					(marisa_topleft.y + 64),
					angle_26D87,
					BG_1,
					to_sp(3.125f)
				);
			}
		} else {
			angle_26D87 += 0x0A;
			bullets_add_pellet(
				(marisa_topleft.x + 64),
				(marisa_topleft.y + 16),
				angle_26D87,
				BG_1,
				to_sp(3.75f)
			);
			if(boss_phase_frame > 270) {
				boss_phase_frame = 0;
				patnum_2064E = 128;
			}
		}
	}
}

void near marisa_1B7D3(void)
{
	screen_y_t top;
	register int i;
	register screen_x_t left;

	if(boss_phase_frame == 10) {
		marisa_bit_speed_set(3, -2);
	}
	if(boss_phase_frame < 50) {
		return;
	}
	if(boss_phase_frame < 130) {
		for(i = 0; i < BIT_COUNT; i += 2) {
			marisa_bit_radius[i] += 2;
		}
	} else if(boss_phase_frame < 260) {
	} else if(boss_phase_frame < 340) {
		for(i = 0; i < BIT_COUNT; i += 2) {
			marisa_bit_radius[i] -= 2;
		}
		if(rank != RANK_EASY) {
			for(i = 1; i < BIT_COUNT; i += 2) {
				marisa_bit_radius[i]++;
			}
		}
	} else if(boss_phase_frame < 420) {
		if(rank != RANK_EASY) {
			for(i = 1; i < BIT_COUNT; i += 2) {
				marisa_bit_radius[i]--;
			}
		}
	} else {
		marisa_bit_speed_set(2, -2);
		boss_phase_frame = 0;
		patnum_2064E = 128;
	}
	marisa_1B665(140);
	if(boss_phase_frame == 140) {
		angle_26D88 = 0;
	}
	if(marisa_bit_state_sum == 8) {
		patnum_2064E = 130;
		angle_26D88 -= (0x18 - rank);
		bullets_add_16x16(
			(marisa_topleft.x + 64),
			(marisa_topleft.y + 16),
			angle_26D88,
			BSM_1,
			PAT_BULLET16_STAR,
			to_sp(3.75f)
		);
	} else if(
		(boss_phase_frame >= 150) &&
		(boss_phase_frame < 250) &&
		((boss_phase_frame & 3) == 0)
	) {
		angle_26D88 += 3;
		snd_se_play(10);
		for(i = 0; i < BIT_COUNT; i += 2) {
			if(marisa_bit_state[i] == 0) {
				left = *marisa_bit_left_on_back_page[i];
				top = *marisa_bit_top_on_back_page[i];
				if((left > 0) && (left < 0x1A0) && (top >= 0) && (top <= 0x170)) {
					bullets_add_pellet(
						(left + 12),
						(top + 12),
						(marisa_bit_angle[i] + angle_26D88),
						BG_1,
						to_sp(4.375f)
					);
				}
			}
		}
	}
}

void near marisa_1B996(void)
{
	screen_y_t top;
	int angle;
	register int i;
	register screen_x_t left;

	if(boss_phase_frame == 10) {
		marisa_bit_speed_set(-3, -3);
	}
	if(marisa_bit_state_sum == 8) {
		patnum_2064E = 128;
		boss_phase_frame = 0;
	}
	if(boss_phase_frame < 50) {
		return;
	}
	if(boss_phase_frame == 50) {
		marisa_bit_speed_set(-4, -4);
		snd_se_play(9);
		patnum_2064E = 130;
	} else if(boss_phase_frame == 100) {
		snd_se_play(3);
		for(i = 0; i < BIT_COUNT; i++) {
			if(marisa_bit_state[i] == 0) {
				left = (*marisa_bit_left_on_back_page[i] + 12);
				top = (*marisa_bit_top_on_back_page[i] + 12);
				for(angle = -0x40; angle < 0x40; angle += 0x10) {
					bullets_add_pellet(
						left,
						top,
						(marisa_bit_angle[i] + angle),
						BG_1,
						to_sp(4.375f)
					);
				}
			}
		}
	} else if(boss_phase_frame == 110) {
		snd_se_play(3);
		for(i = 0; i < BIT_COUNT; i++) {
			if(marisa_bit_state[i] == 0) {
				left = (*marisa_bit_left_on_back_page[i] + 12);
				top = (*marisa_bit_top_on_back_page[i] + 12);
				for(angle = -0x46; angle < 0x46; angle += 0x0C) {
					bullets_add_pellet(
						left,
						top,
						(marisa_bit_angle[i] + angle),
						BG_1,
						(randring2_next8_and(0x1F) + to_sp(1.0f))
					);
				}
				marisa_bit_state[i] = 1;
			}
		}
	}
}

void near marisa_1BAFF(void)
{
	screen_y_t top;
	subpixel_t speed;
	register int i;
	register screen_x_t left;

	if(boss_phase_frame < 50) {
		return;
	}
	if(boss_phase_frame >= 150) {
		marisa_1B665(120);
	}
	if(boss_phase_frame == 50) {
		snd_se_play(9);
		patnum_2064E = 130;
		byte_26D89 = 0;
		marisa_bit_spin[0] = 0;
		marisa_bit_spin[1] = 0;
		marisa_bit_spin[2] = 0;
		marisa_bit_spin[3] = 0;
	} else if(boss_phase_frame == 300) {
		boss_phase_frame = 0;
		marisa_bit_speed[0] *= -1;
		marisa_bit_speed[1] *= -1;
		marisa_bit_speed[2] *= -1;
		marisa_bit_speed[3] *= -1;
	}
	if((boss_phase_frame > 70) && ((boss_phase_frame % 5) == 0)) {
		snd_se_play(10);
		for(i = 0; i < BIT_COUNT; i++) {
			if(marisa_bit_state[i] == 0) {
				left = (*marisa_bit_left_on_back_page[i] + 12);
				top = (*marisa_bit_top_on_back_page[i] + 12);
				speed = ((boss_phase_frame >> 2) + to_sp(1.5625f));
				bullets_add_pellet(
					left,
					top,
					(marisa_bit_angle[i] + marisa_bit_spin[i]),
					BG_1,
					speed
				);
				marisa_bit_spin[i] += (marisa_bit_speed[i] * 2);
			}
		}
		if((marisa_bit_state_sum == 8) && ((byte_26D89 & 1) == 0)) {
			bullets_add_pellet(
				(marisa_topleft.x + 44),
				(marisa_topleft.y + 44),
				boss_phase_frame,
				BG_16_RING,
				to_sp(3.125f)
			);
		}
		byte_26D89++;
	}
}

#define marisa_star_add(left, top, angle, group) { \
	bullets_add_16x16( \
		left, top, angle, group, PAT_BULLET16_STAR, to_sp(4.375f) \
	); \
}

void near marisa_1BC43(void)
{
	int i;
	unsigned char angle;
	register screen_x_t edge_left;

	if(boss_phase_frame < 50) {
		return;
	}
	if(boss_phase_frame >= 150) {
		marisa_1B665(120);
	}
	if(boss_phase_frame == 50) {
		snd_se_play(9);
		patnum_2064E = 130;
		bullet_special.u1.drift_angle = byte_1EEA5;
		bullet_special.u2.drift_speed.v = 0;
		bullet_special.u3.drift_frames = 64;
		byte_1EEA5 *= -1;
		marisa_bit_speed_set(2, -2);
	} else if(boss_phase_frame == 70) {
		marisa_bit_speed_set(3, -3);
	} else if(boss_phase_frame == 80) {
		marisa_bit_speed_set(4, -4);
	} else if(boss_phase_frame == 200) {
		marisa_bit_speed_set(2, -2);
		boss_phase_frame = 0;
	}
	if((boss_phase_frame > 70) && ((boss_phase_frame % 14) == 0)) {
		snd_se_play(10);
		if(boss_phase_frame <= 120) {
			for(i = 0; i < BIT_COUNT; i++) {
				if(marisa_bit_state[i] == 0) {
					marisa_star_add(
						(*marisa_bit_left_on_back_page[i] + 8),
						(*marisa_bit_top_on_back_page[i] + 8),
						-0x40,
						BSM_1
					);
				}
			}
			marisa_star_add(
				(marisa_topleft.x + 40), (marisa_topleft.y + 40), -0x40, BSM_1
			);
		} else {
			if(byte_1EEA5 == 1) {
				edge_left = 0x190;
			} else {
				edge_left = 0x20;
			}
			for(i = 0; byte_20671 > i; i++) {
				angle = (randring2_next8_and(7) + (byte_1EEA5 << 4) + 0x40);
				bullets_add_16x16(
					((randring2_next16() % (PLAYFIELD_W - BULLET16_W)) + PLAYFIELD_LEFT),
					PLAYFIELD_TOP,
					angle,
					BSM_1,
					PAT_BULLET16_STAR,
					(randring2_next8_and(0x1F) + to_sp(1.0f))
				);
				bullets_add_16x16(
					edge_left,
					((randring2_next16() % 320) + PLAYFIELD_TOP),
					angle,
					BSM_1,
					PAT_BULLET16_STAR,
					(randring2_next8_and(0x1F) + to_sp(1.0f))
				);
			}
		}
		if(marisa_bit_state_sum == 8) {
			for(i = 0; i < BIT_COUNT; i++) {
				marisa_star_add(
					(marisa_topleft.x + 40),
					(marisa_topleft.y + 40),
					((i << 6) + (boss_phase_frame * 2)),
					BSM_DRIFT_ANGLE_CHASE
				);
			}
		}
	}
}

void near marisa_1BE72(void)
{
	if(boss_phase_frame == 1) {
		return;
	}
	if(boss_phase_frame == 2) {
		word_26D8E = ((marisa_topleft.x + 32) < player_topleft.x) ? 1 : -1;
		word_26D90 = (marisa_topleft.y < 72) ? 1
			: (marisa_topleft.y > 108) ? -1
			: (1 - (randring2_next8() % 3));
	}
	if(boss_phase_frame < 50) {
		*boss_left_on_back_page += word_26D8E;
		*boss_top_on_back_page += word_26D90;
		marisa_topleft.x = *boss_left_on_back_page;
		marisa_topleft.y = *boss_top_on_back_page;
	}
}

int marisa_update(void)
{
	register int i;
	register screen_x_t left;

	for(i = 0, marisa_bit_state_sum = 0; i < BIT_COUNT; i++) {
		marisa_bit_state_sum += marisa_bit_state[i];
	}
	if(marisa_bit_state_sum == 8) {
		shot_target_left = (marisa_topleft.x + 40);
		shot_target_top = (marisa_topleft.y + 40);
	} else {
		shot_target_left = -1;
		shot_target_top = -1;
	}
	boss_phase_frame++;
	if((stage_frame & 1) && (!reduce_effects || (stage_frame & 3))) {
		left = ((randring2_next16() % 384) + 32);
		sub_3E1C(left, 16, (((0xE0 - left) / 3) + 0x40));
	}
	if((stage_frame & 0x1F) == 0) {
		marisa_bg_col_p++;
		if(marisa_bg_col_p >= 6) {
			marisa_bg_col_p = 0;
		}
		byte_1FFF8 = MARISA_BG_COLORS[marisa_bg_col_p];
	}
	sub_3F8A();
	if(!byte_2066A) {
		if(marisa_phase == 0) {
			marisa_1B24A();
			if(boss_phase_frame == 0) {
				marisa_phase++;
			}
		} else if(marisa_phase == 1) {
			marisa_1B477();
			if(boss_phase_frame == 0) {
				marisa_phase++;
				marisa_pattern = 1;
				marisa_pattern_rounds = 0;
			}
		} else if(marisa_phase == 2) {
			switch(marisa_pattern) {
			case -3:	marisa_1B35F();	marisa_1BE72();	break;
			case -2:	marisa_1B2E9();	marisa_1BE72();	break;
			case -1:	marisa_1B24A();	marisa_1BE72();	break;
			case 0: 	marisa_1B477();	break;
			case 1: 	marisa_1B555();	break;
			case 2: 	marisa_1B6DA();	marisa_1BE72();	break;
			case 3: 	marisa_1BAFF();	marisa_1BE72();	break;
			case 4: 	marisa_1BC43();	marisa_1BE72();	break;
			case 5: 	marisa_1B7D3();	marisa_1BE72();	break;
			case 6: 	marisa_1B996();	marisa_1BE72();	break;
			case 0x7F:
				marisa_1BE72();
				if(boss_phase_frame > 30) {
					if(marisa_bit_state_sum < 8) {
						if(marisa_pattern_rounds >= 4) {
							marisa_pattern = 6;
						} else {
							marisa_pattern = ((randring2_next8() % 5) + 1);
							marisa_pattern_rounds++;
						}
					} else if(marisa_pattern_cycle >= 2) {
						marisa_pattern_rounds = 0;
						marisa_pattern = 0;
						marisa_pattern_cycle = 0;
						marisa_bit_patterns_seen++;
						if(marisa_bit_patterns_seen >= 2) {
							byte_1EE96 = 1;
						}
						if(marisa_bit_patterns_seen >= 7) {
							byte_2066A = true;
						}
					} else {
						marisa_pattern_cycle++;
						marisa_pattern = (0xFF - (randring2_next8() % 3));
					}
					boss_phase_frame = 1; // Skip the initial movement
				}
				break;
			}
			if(boss_phase_frame == 0) {
				marisa_pattern = 0x7F;
			}
		}
	}
	marisa_1AA60();
	marisa_1AB35();
	if(byte_2066A) {
		if(marisa_1AC7B()) {
			return SP_CLEAR;
		}
	} else {
		marisa_1AE98();
	}
	for(i = 0; i < BIT_COUNT; i++) {
		if(marisa_bit_state[i] == 1) {
			marisa_1AD80(i);
		}
	}
	marisa_1B3DE();
	marisa_1B025();
	return SP_BOSS;
}

void marisa_bg_render(void)
{
	register int i;

	boss_left_on_back_page = &boss_left_on_page[page_back];
	boss_top_on_back_page = &boss_top_on_page[page_back];
	egc_off();
	grcg_setcolor(GC_RMW, 0);
	grcg_byteboxfill_x(
		PLAYFIELD_VRAM_LEFT,
		PLAYFIELD_TOP,
		(PLAYFIELD_VRAM_RIGHT - 1),
		(PLAYFIELD_BOTTOM - 1)
	);
	sub_3EF4();
	*boss_left_on_back_page = boss_left_on_page[page_front];
	*boss_top_on_back_page = boss_top_on_page[page_front];
	for(i = 0; i < BIT_COUNT; i++) {
		marisa_bit_left_on_back_page[i] = &marisa_bit_left[page_back][i];
		marisa_bit_top_on_back_page[i] = &marisa_bit_top[page_back][i];
		if(marisa_bit_state[i] < 2) {
			*marisa_bit_left_on_back_page[i] = marisa_bit_left[page_front][i];
			*marisa_bit_top_on_back_page[i] = marisa_bit_top[page_front][i];
		}
	}
	grcg_off();
}

void marisa_end(void)
{
	dialog_pre();
	dialog_script_stage4_post_animate();
	stage_clear_bonus_animate();
	overlay_stage_leave_animate();
	stage_id++;
}
