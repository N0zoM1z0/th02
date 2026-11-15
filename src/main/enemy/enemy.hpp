// Stage enemies and the formations that spawn them
// ------------------------------------------------

#include "pc98.h"

struct stage_formation_t {
	int16_t unknown_00[2];
	screen_point_t bullet_offset;
	int16_t patnum;
	int16_t cels;
	int16_t frames_per_cel;
	int16_t kill_patnum_offset_unused; // 0 selects the regular kill sprites
	uint8_t item;
	int8_t unknown_11;
	int32_t score;
	int16_t hp; // -1 = not hittable, -2 = invincible
	bool player_collision_disabled;
	int8_t unknown_19;
	int16_t speed; // ACTUAL MEANING: Frames between aimed pellets.

	// Not part of the file.
	pixel_t w;
	pixel_t h;
	unsigned char far *script;
};

static const int STAGE_FORMATION_COUNT = 128;

extern stage_formation_t stage_formations[STAGE_FORMATION_COUNT];

enum enemy_flag_t {
	EF_FREE = 0,
	EF_ALIVE = 1,
	EF_REMOVE = 2, // Unblitted on the next frame, then freed
};

struct enemy_t {
	screen_point_t pos[PAGE_COUNT];
	int16_t script_ip;
	int16_t kill_frame; // Also the frame counter of waiting script commands
	int16_t formation;
	uint8_t flag;
	uint8_t age;
	bool dying;
	int8_t unknown_11;
	int16_t cel;
	int16_t anim_mode; // 1 = cycles [cel], 2 = [cel] is the patnum
	int16_t loop_i; // Also used as an angle
	int16_t spawned_left; // Spawned on the left half of the playfield?
	int16_t loop_j;
	int8_t velocity_x;
	int8_t velocity_y;
	bool clip_y;
	int8_t unknown_1F;
	int16_t damage;
	bool hittest_disabled;
	bool player_collision_disabled;
	uint8_t bullet_group;
	uint8_t bullet_speed;
};

static const int ENEMY_COUNT = 25;

extern enemy_t enemies[ENEMY_COUNT];

// Number of enemy slots that might be in use.
extern uint8_t enemy_count;

// State of the enemy currently being updated
// ------------------------------------------

extern enemy_t near *enemy_cur;
extern stage_formation_t near *enemy_formation;
extern screen_x_t near *enemy_left_on_back_page;
extern screen_y_t near *enemy_top_on_back_page;
extern screen_x_t enemy_left;
extern screen_y_t enemy_top;
extern int8_t near *enemy_velocity_x;
extern int8_t near *enemy_velocity_y;
// ------------------------------------------
