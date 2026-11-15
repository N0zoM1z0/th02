#include "th02/hiscore/regist.cpp"

#include "th01/rank.h"
#include "th02/main/score.hpp"

// The initializers stay in the original data order of the assembly part.
struct game_clear_constants_t {
	int16_t v[SHOTTYPE_COUNT];
};
struct extra_clear_flags_t {
	uint8_t v[SHOTTYPE_COUNT];
};

// Uninitialized data, in the original order
#pragma option -a2
score_t hiscore;
uint8_t hiscore_continues;
scoredat_section_t hi;
#pragma option -a-

// Data, in the original order
// ---------------------------

const char *SCOREDAT_FN_PTR = SCOREDAT_FN;

unsigned char gbHI_SCORE[] = {
	gb_H, gb_I, gb__, gb_S, gb_C, gb_O, gb_R, gb_E, 0
};
extern const unsigned char gALPHABET[ALPHABET_ROWS][ALPHABET_COLS] = {
	gb_A, gb_B, gb_C, gb_D, gb_E, gb_F, gb_G, gb_H, gb_I,
	gb_J, gb_K, gb_L, gb_M, gb_N, gb_O, gb_P, gb_Q,

	gb_R, gb_S, gb_T, gb_U, gb_V, gb_W, gb_X, gb_Y, gb_Z,
	gs_BULLET, gs_PERIOD, gs_EXCLAMATION, gs_QUESTION, gs_ELLIPSIS,
	gs_HEART, gs_YINYANG, gs_BOMB,

	gb_0, gb_1, gb_2, gb_3, gb_4, gb_5, gb_6, gb_7, gb_8, gb_9,
	gs_SKULL, gs_GHOST, gs_SIDDHAM_HAM, gs_SPACE,
	gs_ARROW_LEFT, gs_ARROW_RIGHT, gs_END,
};
unsigned char gbNAME[] = { gb_N, gb_A, gb_M, gb_E, 0 };
unsigned char gbPOINT[] = { gb_P, gb_O, gb_I, gb_N, gb_T, 0 };
unsigned char gbST[] = { gb_S, gb_T, 0 };

extern const game_clear_constants_t GAME_CLEAR_CONSTANTS_DATA = {
	GAME_CLEAR_CONSTANTS
};
extern const extra_clear_flags_t EXTRA_CLEAR_FLAGS_DATA = { EXTRA_CLEAR_FLAGS };

void hiscore_init(void)
{
	scoredat_init();
	hiscore = ((hi.score.score[0] / 10) >= score)
		? (hi.score.score[0] / 10)
		: score;
	hiscore_continues = (hi.score.score[0] % 10);
}

void scoredat_game_clear_save(void)
{
	game_clear_constants_t game_clear_constants = GAME_CLEAR_CONSTANTS_DATA;
	char rank_save = rank;
	rank = resident->shottype;
	scoredat_load();
	hi.score.cleared = game_clear_constants.v[rank];
	scoredat_save();
	rank = rank_save;
}

void scoredat_extra_clear_save(void)
{
	extra_clear_flags_t extra_clear_flags = EXTRA_CLEAR_FLAGS_DATA;
	char rank_save = rank;
	rank = RANK_LUNATIC;
	scoredat_load();
	hi.score.cleared |= extra_clear_flags.v[resident->shottype];
	scoredat_save();
	rank = rank_save;
}
