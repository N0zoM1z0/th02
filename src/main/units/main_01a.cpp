/* ReC98
 * -----
 * Stage initialization and the pause menu of TH02's MAIN.EXE, the first
 * translation unit of code segment #1 decompiled after main()
 */

#pragma option -zCmain_01_TEXT -zPmain_01

#include <mem.h>
#include "platform.h"
#include "x86real.h"
#include "pc98.h"
#include "libs/master.lib/master.hpp"
#include "libs/master.lib/pc98_gfx.hpp"
#include "th02/resident.hpp"
#include "th02/gaiji/gaiji.h"
#include "th02/hardware/frmdelay.h"
#include "th02/hardware/input.hpp"
#include "th02/hardware/pages.hpp"
#include "th02/formats/map.hpp"
#include "th02/formats/mpn.hpp"
#include "th02/formats/pi.h"
#include "th02/math/randring.hpp"
#include "th02/snd/snd.h"
#include "th02/main/frames.hpp"
#include "th02/main/score.hpp"
#include "th02/main/scroll.hpp"
#include "th02/main/slowdown.hpp"
#include "th02/main/bullet/bullet.hpp"
#include "th02/main/dialog/dialog.hpp"
#include "th02/main/hud/hud.hpp"
#include "th02/main/hud/overlay.hpp"
#include "th02/main/item/item.hpp"
#include "th02/main/player/player.hpp"
#include "th02/main/stage/stage.hpp"
#include "th02/main/tile/tile.hpp"
#include "th02/shiftjis/fns.hpp"
#include "th02/shiftjis/main.hpp"
#include "th02/main/midboss/midboss.hpp"
#include "th01/rank.h"
#include "th02/core/globals.hpp"
#include "th02/core/zunerror.h"
#include "th02/gaiji/loadfree.h"
#include "th02/main/playperf.hpp"

static const unsigned char CONTINUE_INVINCIBILITY_FRAMES = 200;

#include "th02/main/null.hpp"
#include "decomp.hpp"
#include "platform/x86real/pc98/egc.hpp"
#include "th02/main/main.hpp"
#include "th02/main/spark.hpp"
#include "th02/main/player/bomb.hpp"
#define DEMO_N 7000 /* ZUN symbol [MAGNet2010] */
extern input_t *DemoBuf; /* ZUN symbol [MAGNet2010] */
#include "th01/hardware/egc_impl.hpp"

// Data, in the original order
// ---------------------------

#pragma option -a2


unsigned char gStage1[16] = { gb_S, gb_T, gb_A, gb_G, gb_E, gb_1 };
unsigned char gEXTRA_STAGE[16] = {
	gb_E, gb_X, gb_T, gb_R, gb_A, gb__, gb__, gb_S, gb_T, gb_A, gb_G, gb_E
};
const char near *BGM_TITLES[] = {
	BGM_TITLE_02, BGM_TITLE_03, BGM_TITLE_04, BGM_TITLE_05,
	BGM_TITLE_06, BGM_TITLE_07, BGM_TITLE_08, BGM_TITLE_09,
	BGM_TITLE_10, BGM_TITLE_11, BGM_TITLE_12, BGM_TITLE_13,
};
const char near *STAGE_TITLES[] = {
	STAGE1_TITLE, STAGE2_TITLE, STAGE3_TITLE,
	STAGE4_TITLE, STAGE5_TITLE, EXTRA_TITLE,
};
uint8_t STAGE_TITLE_HALFLENGTHS[] = { 10, 13, 11, 12, 13, 13 };
unsigned char gDEMO_PLAY[] = {
	gb_D, gb_E, gb_M, gb_O, gb__, gb_P, gb_L, gb_A, gb_Y, 0
};
unsigned char gPAUSE_MENU[] = {
	gb_P, gb_A, gb_U, gb_S, gb_E, gb__, gb_M, gb_E, gb_N, gb_U, 0
};
unsigned char g11SPACES[] = {
	gb__, gb__, gb__, gb__, gb__, gb__, gb__, gb__, gb__, gb__, gb__, 0
};

typedef void (far *voidfunc_t)(void);
typedef int (far *intfunc_t)(void);

extern voidfunc_t boss_bg_render;
extern intfunc_t boss_update;
extern voidfunc_t boss_init;
extern voidfunc_t boss_end;
extern voidfunc_t boss_bg_render_func;
extern intfunc_t boss_update_func;
extern intfunc_t farfp_1F48C;
extern voidfunc_t farfp_1F490;
extern voidfunc_t farfp_1F494;
extern voidfunc_t farfp_1F498;
extern voidfunc_t farfp_1F4A0;
extern voidfunc_t farfp_23A72;
extern voidfunc_t farfp_23A76;
extern voidfunc_t farfp_26C3C;
extern voidfunc_t farfp_26C40;

extern int midboss_tile_row;
extern uint8_t bgm_show_timer;
extern uint8_t bgm_title_id;
extern uint8_t byte_1F46E;
extern uint8_t stage1_gaiji_halflen;
extern const char near *stage_title;
extern uint8_t stage_title_halflen;
extern uint8_t stage_palette[sizeof(Palette8)];

#define BOSS_DEC(name) \
	void name##_init(void); \
	void name##_end(void); \
	void name##_bg_render(void); \
	int name##_update(void);

BOSS_DEC(rika);
BOSS_DEC(meira);
BOSS_DEC(stones);
BOSS_DEC(marisa);
BOSS_DEC(mima);
BOSS_DEC(sigma);

void sub_17979(void);
void sub_1766E(void);
void sub_BFD0(void);
#define EMPTY_LINE "                                                "

void sub_C05D(void);
void sub_13671(void);
void sub_140AE(void);
void sub_13513(void);
void sub_10E95(void);
void sub_10E4F(void);
void sub_19E2F(void);
void sub_129FC(void);
void sub_13ABB(char far *fn);
void sub_16A6B(void);
void sub_3DDE(void);
void sub_129DD(void);
void near sub_E271(void);
void sub_1028C(void);
void sub_CA1C(void);
void stage_data_free(void);
void stage_data_load(void);
void near sub_C5B0(void);
extern "C" void pascal near text_wipe(void);

extern intfunc_t farfp_1F4A4;

// Uninitialized data, in the original order
// -----------------------------------------

stage_progression_t stage_progression;
uint8_t stage1_gaiji_halflen;
uint8_t stage_title_halflen;
const char near *stage_title;
uint8_t bgm_show_timer;
uint8_t bgm_title_id;
uint8_t byte_1F46E;
void (*midboss_update_and_render)(void);
bool (*midboss_invalidate)(void);

// *_func() functions are "activated" by setting the regular function once the
// boss battle starts.
voidfunc_t boss_bg_render;
intfunc_t boss_update;
intfunc_t boss_update_func;
voidfunc_t boss_init;
voidfunc_t boss_end;

intfunc_t farfp_1F48C;
voidfunc_t farfp_1F490;
voidfunc_t farfp_1F494;
voidfunc_t farfp_1F498;
voidfunc_t boss_bg_render_func;
voidfunc_t farfp_1F4A0;
intfunc_t farfp_1F4A4;
input_t *DemoBuf; /* ZUN symbol [MAGNet2010] */
uint8_t playperf_max;
// ZUN bloat: Not a Palette8, as that would be word-aligned under -a2.
uint8_t stage_palette[sizeof(Palette8)];
unsigned char pause_col_first;
unsigned char pause_col_second;
// -----------------------------------------

int near cfg_load(void);
void hiscore_init(void);
void near sub_E178(void);
extern input_t *DemoBuf;
int game_init_main(void);
void demo_load(void);
int stage_loop(void);
int near gameover_and_continue_menu(void);
int GameExecl(char *binary_fn);
void near main_init(void);
void near stage_init(void);
void near stage_continue_reset(void);

int main(void)
{
	if(!cfg_load()) {
		return 1;
	}
	if(game_init_main()) {
		zun_error(ERROR_OUT_OF_MEMORY);
		return 1;
	}
	if(resident->bgm_mode == SND_BGM_FM) {
		snd_pmd_resident();
		snd_midi_active = false;
		snd_determine_mode();
	} else if(resident->bgm_mode == SND_BGM_MIDI) {
		snd_pmd_resident();
		snd_mmd_resident();
		snd_midi_active = snd_midi_possible;
		snd_determine_mode();
	}
	if(resident->demo_num != 0) {
		demo_load();
	}
	main_init();

next_stage:
	random_seed = resident->frame;
	stage_init();
	overlay_stage_enter_animate();
	if(resident->demo_num == 0) {
		if(stage_id == 5) {
			gaiji_putsa(16, 12, gEXTRA_STAGE, TX_YELLOW);
		} else {
			gaiji_putsa((28 - stage1_gaiji_halflen), 12, gStage1, TX_YELLOW);
		}
		text_putsa((28 - stage_title_halflen), 13, stage_title, TX_WHITE);
	} else {
		gaiji_putsa(18, 12, gDEMO_PLAY, (TX_YELLOW | TX_BLINK));
	}
	key_det = 0;
	while(1) {
		if(farfp_1F4A4()) {
			snd_kaja_func(KAJA_SONG_FADE, 40);
			goto next_stage;
		}
		if(resident->demo_num == 0) {
			if(gameover_and_continue_menu()) {
				stage_continue_reset();
				continue;
			}
			resident->stage = stage_id;
		} else {
			HMem<input_t>::free(DemoBuf);
		}
		break;
	}
	PaletteTone = 0;
	palette_show();
	GameExecl(BINARY_OP);
	return 0;
}

void near main_init(void)
{
	int i;

	snd_load("huuma.efc", SND_LOAD_SE);
	hiscore_init();
	pi_load(0, "EYE.PI");
	super_entry_bfnt("miko.bft");
	super_entry_bfnt("miko32.bft");
	super_entry_bfnt("miko16.bft");
	for(i = 48; i < 128; i++) {
		super_convert_tiny(i);
	}
	gaiji_load();
	sub_E178();
	reduce_effects = resident->reduce_effects;
	farfp_1F4A4 = stage_loop;
	if(resident->continues_used != 0) {
		score = (resident->continues_used >= 9) ? 9 : resident->continues_used;
		item_bigpower_override = ((stage_id % 5) + 2);
	}
	if(rank == RANK_EASY) {
		playperf_max = 4;
	} else {
		playperf_max = 16;
	}
}

void pascal near stage_fn_set_ext(char far *fn, const char far *ext)
{
	fn[7] = ext[0];
	fn[8] = ext[1];
	fn[9] = ext[2];
}

void pascal near stage_fn_init(char far *fn)
{
	fn[0] = 's';
	fn[1] = 't';
	fn[2] = 'a';
	fn[3] = 'g';
	fn[4] = 'e';
	fn[5] = ('0' + stage_id);
	fn[6] = '.';
	fn[7] = 'b';
	fn[8] = 'f';
	fn[9] = 't';
	fn[10] = '\0';
}

#define boss_funcs_set(name) { \
	boss_init = name##_init; \
	boss_end = name##_end; \
	boss_bg_render_func = name##_bg_render; \
	boss_update_func = name##_update; \
}

void near stage_init(void)
{
	char fn[12];
	int i;

	stage_fn_init(fn);
	vsync_Count1 = 0;
	text_wipe();
	graph_scrollup(0);
	graph_accesspage(1);
	graph_clear();
	graph_accesspage(0);
	graph_clear();
	graph_showpage(0);
	hud_put();
	overlay_wipe();
	pi_palette_apply(0);
	pi_palette_apply(0);
	pi_put_8(96, 144, 0);
	bullets_and_sparks_init();
	sub_16A6B();
	sub_3DDE();
	sub_129DD();
	sub_E271();
	snd_se_reset();
	sub_1028C();
	sub_CA1C();
	randring_fill();
	PaletteTone = 100;
	palette_show();
	player_left_on_page[1] = player_left_on_page[0] = PLAYER_LEFT_START;
	player_top_on_page[1] = player_top_on_page[0] = PLAYER_TOP_START;
	stage_frame = 0;
	midboss_active = false;
	stage_progression = SP_STAGE;
	slowdown_factor = 1;
	if(resident->demo_num == 0) {
		bgm_show_timer = 1;
		bgm_title_id = (stage_id * 2);
		byte_1F46E = ((stage_id * 2) + 1);
	} else {
		bgm_show_timer = 0;
	}
	for(i = 0xC0; i >= 0x80; i--) {
		if(super_patsize[i]) {
			super_cancel_pat(i);
		}
	}
	stage1_gaiji_halflen = 6;
	gStage1[5] = ((stage_id % 5) + gb_1);
	stage_title = STAGE_TITLES[stage_id];
	stage_title_halflen = STAGE_TITLE_HALFLENGTHS[stage_id];
	super_entry_bfnt(fn);
	stage_fn_set_ext(fn, "bmt");
	super_entry_bfnt(fn);
	stage_fn_set_ext(fn, "bbt");
	super_entry_bfnt(fn);
	stage_fn_set_ext(fn, "map");
	map_load(fn);
	tiles_stuff_reset();
	stage_fn_set_ext(fn, "mpn");
	mpn_load(fn);
	memcpy(&stage_palette, &mpn_palette, sizeof(Palette8));
	stage_data_free();
	stage_data_load();
	dialog_load_and_init();
	boss_bg_render = nullfunc_void;
	boss_update = nullfunc_false;
	farfp_1F490 = nullfunc_void;
	farfp_1F4A0 = nullfunc_void;
	farfp_23A72 = nullfunc_void;
	farfp_23A76 = nullfunc_void;
	farfp_26C3C = sub_17979;
	farfp_26C40 = sub_1766E;
	farfp_1F494 = sub_BFD0;
	farfp_1F498 = sub_C05D;
	farfp_1F48C = nullfunc_false;
	sub_C5B0();
	scroll_speed = 1;
	scroll_interval = 4;
	switch(stage_id) {
	case 0:
		midboss_tile_row = 116;
		midboss_invalidate = midboss1_invalidate;
		midboss_update_and_render = midboss1_update_and_render;
		boss_funcs_set(rika);
		farfp_1F490 = sub_13671;
		farfp_1F4A0 = sub_13513;
		break;
	case 1:
		midboss_tile_row = 80;
		midboss_invalidate = midboss2_invalidate;
		midboss_update_and_render = midboss2_update_and_render;
		boss_funcs_set(meira);
		farfp_1F490 = sub_140AE;
		farfp_1F4A0 = sub_13513;
		break;
	case 2:
		midboss_tile_row = 103;
		midboss_invalidate = midboss3_invalidate;
		midboss_update_and_render = midboss3_update_and_render;
		boss_funcs_set(stones);
		farfp_1F490 = sub_10E95;
		farfp_1F4A0 = sub_10E4F;
		break;
	case 3:
		midboss_tile_row = 944;
		midboss_invalidate = midboss4_invalidate;
		midboss_update_and_render = midboss4_update_and_render;
		boss_funcs_set(marisa);
		if(!reduce_effects) {
			farfp_1F490 = sub_19E2F;
		}
		sub_129FC();
		scroll_speed = 2;
		scroll_interval = 1;
		break;
	case 4:
		midboss_tile_row = -1;
		midboss_invalidate = reinterpret_cast<bool (*)(void)>(nullfunc_false);
		midboss_update_and_render = nullfunc_void;
		boss_funcs_set(mima);
		scroll_interval = 1;
		break;
	case 5:
		midboss_tile_row = 200;
		midboss_invalidate = midbossx_invalidate;
		midboss_update_and_render = midbossx_update_and_render;
		boss_funcs_set(sigma);
		sub_129FC();
		scroll_interval = 2;
		break;
	}
	tile_area_init_and_put_both();
	if(resident->demo_num == 0) {
		snd_delay_until_volume(255);
		stage_fn_set_ext(fn, "m\0\0"); // all three bytes are copied
	}
	items_init_and_reset();
	score_extend_init();
	while(vsync_Count1 < 100) {
	}
	text_boxfilla(4, 1, 51, 23, (TX_BLACK | TX_REVERSE));
	memcpy(&Palettes, &stage_palette, sizeof(Palette8));
	palette_show();
	graph_accesspage(1);
	tiles_fill_and_put_initial();
	graph_accesspage(0);
	tiles_render_all();
	mpn_free();
	mpn_load("miko_k.mpn");
	if(resident->demo_num == 0) {
		sub_13ABB(fn);
	}
	grc_setclip(0, 0, (RES_X - 1), (RES_Y - 1));
	grcg_setcolor(GC_RMW, 11);
	graph_accesspage(0);
	grcg_boxfill(PLAYFIELD_RIGHT, 0, 575, (RES_Y - 1));
	graph_accesspage(1);
	grcg_boxfill(PLAYFIELD_RIGHT, 0, 575, (RES_Y - 1));
	grcg_off();
	page_front = 1;
	_AL = page_front;
	asm {
		out 0xA4, al
		xor al, al
		out 0xA6, al
	}
	page_back = _AL;
}

void near stage_continue_reset(void)
{
	hud_put();
	overlay_stage_leave_animate();
	player_left_on_page[0] = player_left_on_page[1] = PLAYER_LEFT_START;
	player_top_on_page[0] = player_top_on_page[1] = PLAYER_TOP_START;
	sub_C5B0();
	player_invincibility_time = CONTINUE_INVINCIBILITY_FRAMES;
	graph_accesspage(page_front);
	tiles_render_all();
	graph_accesspage(page_back);
	tiles_render_all();
	bullets_clear();
	PaletteTone = 100;
	palette_show();
	overlay_stage_enter_animate();
}

const char far *PAUSE_CHOICE_QUIT = PAUSE_CHOICE_QUIT_STR;
const char far *PAUSE_CHOICE_RESUME = PAUSE_CHOICE_RESUME_STR;
unsigned char pause_sel = 0;
const char far *QUIT_PROMPT = QUIT_PROMPT_STR;
const char far *QUIT_CHOICE_NO = QUIT_CHOICE_NO_STR;
const char far *QUIT_CHOICE_YES = QUIT_CHOICE_YES_STR;
extern unsigned char pause_col_first;
extern unsigned char pause_col_second;

#define pause_sel_flip() { \
	pause_sel = (1 - pause_sel); \
	if(pause_sel == 0) { \
		pause_col_first = (TX_WHITE | TX_UNDERLINE); \
		pause_col_second = TX_MAGENTA; \
	} else { \
		pause_col_first = TX_MAGENTA; \
		pause_col_second = (TX_WHITE | TX_UNDERLINE); \
	} \
}

int near pause_menu(void)
{
	unsigned char state;
	unsigned int frame;

	frame = 0;
	state = 0;
	text_putsa(23, 15, PAUSE_CHOICE_RESUME, (TX_WHITE | TX_UNDERLINE));
	text_putsa(23, 16, PAUSE_CHOICE_QUIT, TX_MAGENTA);
	PaletteTone = 70;
	palette_show();
	while(1) {
		if((frame & 0x3F) < 50) {
			gaiji_putsa(18, 12, gPAUSE_MENU, TX_WHITE);
		} else {
			gaiji_putsa(18, 12, g11SPACES, TX_WHITE);
		}
		input_reset_sense();
		frame++;
		if((state == 0) && (key_det == INPUT_NONE)) {
			state = 1;
		}
		if(state == 1) {
			if((key_det & INPUT_UP) || (key_det & INPUT_DOWN)) {
				pause_sel_flip();
				text_putsa(23, 15, PAUSE_CHOICE_RESUME, pause_col_first);
				text_putsa(23, 16, PAUSE_CHOICE_QUIT, pause_col_second);
				state = 0;
			}
			if((key_det & INPUT_OK) || (key_det & INPUT_SHOT)) {
				if(pause_sel == 0) {
					state = 2;
				} else {
					text_putsa(17, 14, QUIT_PROMPT, (TX_WHITE | TX_BLINK));
					text_putsa(17, 15, QUIT_CHOICE_NO, TX_WHITE);
					text_putsa(17, 16, QUIT_CHOICE_YES, TX_MAGENTA);
					state = 3;
					pause_sel = 0;
				}
			}
			if(key_det & INPUT_CANCEL) {
				state = 2;
			}
		} else if(state == 2) {
			if(key_det == INPUT_NONE) {
				PaletteTone = 100;
				palette_show();
				key_det = INPUT_NONE;
				gaiji_putsa(18, 12, g11SPACES, TX_WHITE);
				gaiji_putsa(17, 14, g11SPACES, TX_WHITE);
				gaiji_putsa(17, 15, g11SPACES, TX_WHITE);
				gaiji_putsa(17, 16, g11SPACES, TX_WHITE);
				return 0;
			}
		} else if(state == 3) {
			if(key_det == INPUT_NONE) {
				state++;
			}
		} else if(state == 4) {
			if((key_det & INPUT_UP) || (key_det & INPUT_DOWN)) {
				pause_sel_flip();
				text_putsa(17, 15, QUIT_CHOICE_NO, pause_col_first);
				text_putsa(17, 16, QUIT_CHOICE_YES, pause_col_second);
				state = 3;
			}
			if((key_det & INPUT_OK) || (key_det & INPUT_SHOT)) {
				if(pause_sel == 0) {
					gaiji_putsa(17, 14, g11SPACES, TX_WHITE);
					gaiji_putsa(17, 15, g11SPACES, TX_WHITE);
					gaiji_putsa(17, 16, g11SPACES, TX_WHITE);
					text_putsa(
						23, 15, PAUSE_CHOICE_RESUME, (TX_WHITE | TX_UNDERLINE)
					);
					text_putsa(23, 16, PAUSE_CHOICE_QUIT, TX_MAGENTA);
					state = 0;
				} else {
					return 1;
				}
			}
			if(key_det & INPUT_CANCEL) {
				state = 2;
			}
		}
		frame_delay(1);
	}
}

// The stage loop and its callbacks follow in the same TU, compiled with -G.
#pragma option -G

void bullets_update_and_render(void);
void bullets_invalidate(void);
void near egc_start_copy_1(void);
void near items_invalidate(void);
void near player_update_and_render(void);

extern void (far *farfp_1F498)(void);
extern void (far *farfp_1F4A0)(void);
extern void (far *farfp_23A72)(void);
extern void (far *farfp_23A76)(void);
extern void (far *farfp_26C3C)(void);
extern void (far *farfp_26C40)(void);

// Scroll position for each VRAM page.
struct scroll_line_on_page_t {
	vram_y_t v[PAGE_COUNT];
};
scroll_line_on_page_t SCROLL_LINE_ON_PAGE_INIT = { 0, 0 };

extern uint8_t scroll_speed_this_frame;
extern vram_offset_t scroll_line_vo;
extern int tile_rows_scrolled;
extern int16_t tile_row_scrolled;
extern int midboss_tile_row;

void near bgm_show(void);
void near sub_E2D9(void);
void near player_controls_update(void);
int near pause_menu(void);
void sub_16D9B(void);
extern "C" void pascal DemoPlay(void);

#define egc_off_after_copy() { \
	outport2(EGC_ACTIVEPLANEREG, 0xFFF0); \
	outport2(EGC_MASKREG, 0xFFFF); \
	graph_mode_change(true); \
	graph_mode_egc(false); \
	_outportb_(0x7C, GC_OFF); \
	graph_mode_change(false); \
}

// `JMP SHORT $+2`, as an I/O delay.
#define io_delay() __emit__(0xEB, 0x00)

inline void playperf_raise_over_time(void) {
	if((stage_frame % 1800) == 1500) {
		if(playperf_max > playperf) {
			playperf++;
		}
	}
}

int stage_loop(void)
{
	pixel_t gdc_lines;
	vram_offset_t gdc_start;
	scroll_line_on_page_t scroll_line_on_page = SCROLL_LINE_ON_PAGE_INIT;
	vram_y_t scroll_line_prev;

	while(!quit) {
		snd_se_update();
		farfp_1F498();
		bgm_show();
		scroll_line_prev = scroll_line;
		scroll_line = scroll_line_on_page.v[page_back];
		boss_bg_render();
		if(midboss_active) {
			midboss_active = midboss_invalidate();
		}
		farfp_26C3C();
		player_invalidate();
		bullets_invalidate();
		farfp_23A72();
		items_invalidate();
		sparks_invalidate();
		sub_E2D9();
		if(tile_row_scrolled) {
			sub_16D9B();
			tile_row_scrolled = 0;
		}
		egc_start_copy_1();
		farfp_1F4A0();
		tiles_egc_render();
		farfp_1F490();
		egc_off_after_copy();
		if(tile_rows_scrolled == midboss_tile_row) {
			midboss_active = true;
		}
		scroll_speed_this_frame = 0;
		scroll_line = scroll_line_prev;
		if(!scroll_done && ((scroll_cycle % scroll_interval) == 0)) {
			scroll_line -= scroll_speed;
			if(scroll_line < 0) {
				scroll_line += RES_Y;
			}
			scroll_speed_this_frame = scroll_speed;
		}
		scroll_line_on_page.v[page_back] = scroll_line;
		input_reset_sense();
		if(resident->demo_num != 0) {
			DemoPlay();
		}
		player_controls_update();
		bomb_update_and_render();
		farfp_26C40();
		stage_progression = static_cast<stage_progression_t>(boss_update());
		if(midboss_active) {
			midboss_update_and_render();
		}
		player_update_and_render();
		items_update_and_render();
		farfp_23A76();
		bullets_update_and_render();
		sparks_update_and_render();
		if((key_det & INPUT_CANCEL) && pause_menu()) {
			quit = true;
		}
		if(scroll_speed_this_frame) {
			scroll_line_vo = ((scroll_line << 5) + (scroll_line << 3));
			if((scroll_line & 7) == 0) {
				tile_rows_scrolled++;
				tile_row_scrolled = 1;
			}
			egc_start_copy_1();
			scroll_done = tiles_scroll_and_egc_render_both(scroll_speed);
			egc_off_after_copy();
			if(scroll_done) {
				// ZUN bug: See the comment in the original assembly.
				scroll_line++;
				goto frame_end;
			}
			gdc_lines = (RES_Y - scroll_line);
			gdc_start = scroll_line_vo;
			_CX = graph_VramZoom;
			_CL = 4;
gdc_wait:
			io_delay();
			asm { in al, 0xA0; test al, cl; jz gdc_wait }
			asm { mov al, 0x70; out 0xA2, al }

			asm { mov ax, gdc_start; out 0xA0, al; mov al, ah }
			io_delay();
			io_delay();
			asm { out 0xA0, al }

			asm { mov ax, gdc_lines; shl ax, cl; or ah, ch; out 0xA0, al; mov al, ah }
			io_delay();
			io_delay();
			asm { out 0xA0, al }

			asm { mov ax, 0; out 0xA0, al; mov al, ah }
			io_delay();
			io_delay();
			asm { out 0xA0, al }

			asm { mov ax, RES_Y; shl ax, cl; or ah, ch; out 0xA0, al; mov al, ah }
			io_delay();
			io_delay();
			asm { out 0xA0, al }
			if(scroll_done) {
				goto frame_end;
			}
		}
		scroll_cycle++;
frame_end:
		if(slowdown_factor == 1) {
			vsync_Count1 = 0;
		}
		while(slowdown_factor > vsync_Count1) {
		}
		vsync_Count1 = 0;
		_outportb_(0xA4, page_front = page_back);
		page_back ^= 1;
		_outportb_(0xA6, page_back);
		farfp_1F494();
		resident->frame++;
		stage_frame++;
		if(farfp_1F48C() && !quit) {
			return 1;
		}
		playperf_raise_over_time();
		score_update_and_render();
	}
	score_delta_commit();
	quit = false;
	return 0;
}

// Segment 1, after the stage loop
// -------------------------------

extern "C" void pascal near text_wipe(void)
{
	text_clear();
	text_fillca(' ', (TX_BLACK | TX_REVERSE));
}

void nullfunc_void(void)
{
}

int nullfunc_false(void)
{
	return false;
}

extern uint8_t bgm_show_timer;
extern uint8_t bgm_title_id;
extern uint8_t byte_1F46E;
void sub_17A55(void);
void sub_16A8A(void);

int sub_BF9C(void)
{
	if(stage_progression == SP_CLEAR) {
		if(stage_id == 2) {
			Palettes[0].c.r = 0;
			Palettes[0].c.g = 0;
			Palettes[0].c.b = 0;
			palette_show();
		}
		boss_end();
		return 1;
	}
	return 0;
}

void sub_BFD0(void)
{
	if(scroll_done == true) {
		stage_progression = SP_BOSS;
		if(stage_id == 3) {
			farfp_1F490 = nullfunc_void;
		} else if(stage_id == 2) {
			Palettes[0].c.r = 0;
			Palettes[0].c.g = 0;
			Palettes[0].c.b = 0;
			palette_show();
		}
		boss_init();
		sub_17A55();
		sub_16A8A();
		farfp_1F494 = nullfunc_void;
		boss_bg_render = boss_bg_render_func;
		boss_update = boss_update_func;
		farfp_1F48C = sub_BF9C;
		scroll_cycle = -1;
		bgm_show_timer = 1;
		bgm_title_id = byte_1F46E;
	}
}

void sub_C05D(void)
{
	if(stage_frame == 160) {
		if(resident->demo_num == 0) {
			text_putsa(4, 12, EMPTY_LINE, TX_WHITE);
			text_putsa(4, 13, EMPTY_LINE, TX_WHITE);
		}
		farfp_1F498 = nullfunc_void;
	}
}

void near bgm_show(void)
{
	if(bgm_show_timer) {
		if(bgm_show_timer == 1) {
			gaiji_putca(24, 23, gs_NOTES, TX_YELLOW);
			text_putsa(26, 23, BGM_TITLES[bgm_title_id], TX_WHITE);
		}
		bgm_show_timer++;
		if(bgm_show_timer >= 160) {
			text_putsa(24, 23, (EMPTY_LINE + 22), TX_WHITE);
			bgm_show_timer = 0;
		}
	}
}

void near egc_start_copy_1(void)
{
	egc_on();
	egc_setup_copy();
}


int demo_frame = 0;

void demo_load(void)
{
	DemoBuf = reinterpret_cast<input_t far *>(hmem_allocbyte(DEMO_N * 2));
	power = POWER_MAX;
	playperf = 12;
	resident->frame = 18;
	if(resident->demo_num == 1) {
		stage_id = 3;
		file_ropen("DEMO1.REC");
		resident->shottype = 0;
	} else if(resident->demo_num == 2) {
		stage_id = 2;
		file_ropen("DEMO2.REC");
		resident->shottype = 2;
	} else if(resident->demo_num == 3) {
		stage_id = 1;
		file_ropen("DEMO3.REC");
		resident->shottype = 1;
	}
	file_read(DemoBuf, (DEMO_N * 2));
	file_close();
}

extern "C" void pascal DemoPlay(void)
{
	if(key_det == 0) {
		key_det = DemoBuf[demo_frame];
		demo_frame++;
		if(demo_frame < (DEMO_N - 50)) {
			return;
		}
	}
	key_det = 0;
	palette_black_out(10);
	quit = true;
	snd_se_reset();
}
