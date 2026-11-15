// Included after th02/main/boss/mima.cpp, in the same TU.
#include "th02/main/playperf.hpp"
#include "th02/hardware/frmdelay.h"
#include "th02/main/dialog/dialog.hpp"

void near skill_calculate(void)
{
	int skill = (rank * 20);
	skill += (10 - (resident->continues_used * 3));

	int skill_miss_and_bombs = (50 - (total_miss_count * 3) - total_bombs_used);
	if(skill_miss_and_bombs < 0) {
		skill_miss_and_bombs = 0;
	}
	skill += skill_miss_and_bombs;

	if(item_skill > 25) {
		item_skill = 25;
	} else if(item_skill < 0) {
		item_skill = 0;
	}
	skill += item_skill;
	resident->skill = skill;
}

// Mima's initialization and defeat
// --------------------------------

extern int16_t word_26C66;
extern int16_t word_26C68;
extern int16_t word_26C6A;
extern int16_t word_26CBE;
extern uint8_t byte_26CC0;
extern uint8_t byte_26CC1;
extern uint8_t byte_26CC2;
extern uint8_t byte_26CC3;
extern uint8_t byte_26CC4;
extern uint8_t byte_26CC5;
extern int16_t mima_sphere_active[8];
extern int16_t patnum_2064E;
extern uint8_t byte_2066A;
extern bool byte_2066B;

void near dialog_pre(void);
void near dialog_post(void);
void sub_13ABB(char far *fn);
void shots_clear(void);
void near sub_1A6C5(void);
void near sub_1A46B(void);
void near sub_1A529(void);
void near mima_180AC(void);
void scoredat_game_clear_save(void);
int GameExecl(char *binary_fn);

#define mima_put() { \
	super_put_rect( \
		boss_left_on_page[page_back], boss_top_on_page[page_back], patnum_2064E \
	); \
	super_put_rect( \
		(boss_left_on_page[page_back] + 48), \
		boss_top_on_page[page_back], \
		(patnum_2064E + 1) \
	); \
	super_put_rect( \
		(boss_left_on_page[page_back] + 96), \
		boss_top_on_page[page_back], \
		(patnum_2064E + 2) \
	); \
}

void mima_init(void)
{
	register int i;

	sub_1A6C5();
	dialog_pre();
	super_clean(128, 192);
	super_entry_bfnt("mima.bft");
	dialog_script_stage5_pre_intro_animate();
	vsync_Count1 = 0;
	frame_delay(10);
	sub_1A529();
	super_clean(128, 192);
	super_patnum = 128;
	super_entry_bfnt("mima1.bft");
	super_entry_bfnt("stage3_b.btt");
	boss_left_on_page[0] = (PLAYFIELD_LEFT + (PLAYFIELD_W / 2) - 80);
	boss_left_on_page[1] = boss_left_on_page[0];
	boss_top_on_page[0] = (PLAYFIELD_TOP + 48);
	boss_top_on_page[1] = boss_top_on_page[0];
	patnum_2064E = 128;
	dialog_script_stage5_pre_unsealed_animate();
	palette_white_out(1);
	grc_setclip(PLAYFIELD_LEFT, 0, PLAYFIELD_RIGHT, (RES_Y - 1));
	grcg_setcolor(GC_RMW, 0);
	grcg_fill();
	grcg_off();
	mima_put();
	player_and_options_put();
	snd_se_play(10);
	snd_se_update();
	sub_1A46B();
	palette_white_in(3);
	sub_13ABB("mima.m");
	dialog_script_stage5_pre_winged_animate();
	dialog_post();
	shots_clear();
	boss_damage = 0;
	byte_2066A = false;
	boss_phase_frame = 0;
	byte_2066B = false;
	word_26C68 = 0;
	word_26C66 = 0;
	word_26CBE = 0;
	tile_mode = TM_NONE;
	byte_26CC1 = 0;
	byte_26CC2 = 0;
	byte_26CC3 = 13;
	byte_26CC5 = 3;
	byte_26CC4 = 12;
	graph_accesspage(page_front);
	graph_clear();
	graph_accesspage(page_back);
	graph_clear();
	grcg_setcolor(GC_RMW, 11);
	grc_setclip(PLAYFIELD_RIGHT, 0, (RES_X - 1), (RES_Y - 1));
	graph_accesspage(page_front);
	grcg_fill();
	graph_accesspage(page_back);
	grcg_fill();
	grcg_off();
	grc_setclip(PLAYFIELD_LEFT, 0, PLAYFIELD_RIGHT, (RES_Y - 1));
	word_26C6A = 0;
	for(i = 0; i < 8; i++) {
		mima_sphere_active[i] = 0;
	}
	byte_26CC0 = 0;
	mima_180AC();
}

void near mima_game_end(void)
{
	PaletteTone = 0;
	palette_show();
	resident->score_highest = (resident->score_highest < score)
		? score
		: resident->score_highest;
	resident->score = ((score * 10) + resident->continues_used);
	resident->stage = 0x7F;
	resident->rem_lives = lives;
	resident->rem_bombs = bombs;
	skill_calculate();
	GameExecl("maine");
}

int near mima_19C8D(void)
{
	shots_clear();
	bullets_clear();
	mima_put();
	dialog_pre();
	dialog_script_stage5_form1defeat_animate();
	if(resident->continues_used != 0) {
		snd_kaja_func(KAJA_SONG_FADE, 10);
		palette_white_out(10);
		frame_delay(50);
		score += 50000;
		mima_game_end();
		return 1;
	}
	super_clean(128, 192);
	super_patnum = 128;
	super_entry_bfnt("mima2.bft");
	dialog_post();
	graph_accesspage(page_front);
	grcg_setcolor(GC_RMW, 0);
	grcg_fill();
	graph_accesspage(page_back);
	grcg_fill();
	grcg_off();
	byte_26CC0 = 0;
	return 0;
}

void mima_end(void)
{
	// ZUN bug: This renders Mima's sprite to [page_back] on top of the boss
	// background rendered earlier, right before animating the defeat dialog
	// on [page_front] and launching into MAINE.EXE, which means that none of
	// this will ever show up. Given the fact that this code exists, it
	// probably was ZUN's intention to render the defeat dialog on top of
	// what's rendered here - i.e, before rendering any player shot, item,
	// bullet, or spark sprites - rather than blindly printing the text on top
	// of whatever white VRAM pixels that may have been in the text box area
	// in the previous full frame.
	mima_put();
	dialog_script_stage5_post_animate();
	score += 100000;
	if(resident->continues_used == 0) {
		scoredat_game_clear_save();
	}
	mima_game_end();
}
