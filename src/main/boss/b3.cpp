/// Stage 3 Boss - Five Magic Stones
/// --------------------------------

extern uint8_t stone_unknown[STONE_COUNT];

extern long stone_frames;
extern uint8_t stones_phase;
extern uint8_t stones_pattern;
extern int stones_pattern_frame;
extern screen_x_t stones_bullet_left;
extern screen_y_t stones_bullet_top;
extern vram_y_t stones_y_22D9C;
extern unsigned char angle_1E510;
extern uint8_t pillar_frames_new;

void near dialog_pre(void);
void near dialog_post(void);

void sub_129FC(void);
void sub_13ABB(char far *fn);
void sub_17A55(void);

extern uint8_t byte_2066F;
extern uint8_t group_20670;
extern uint8_t byte_20671;
extern unsigned char angle_22FAF;
extern unsigned char angle_22FB0;
extern unsigned char angle_22FB1[STONE_COUNT];
extern uint8_t stones_ring_tile_flag[TILES_X];
extern uint8_t stones_ring_phase;
extern uint8_t stones_ring_i;
extern uint8_t stones_ring_done;
extern int stones_ring_tile_1;
extern int stones_ring_tile_2;
extern int stones_ring_tile_3;
extern const uint8_t STONES_RING_ORDER_LEFT[TILES_X / 2] = {
	0, 1, 2, 3, 4, 5, 6, 7, 8, 9, 10, 11
};
extern const uint8_t STONES_RING_ORDER_RIGHT[TILES_X / 2] = {
	23, 22, 21, 20, 19, 18, 17, 16, 15, 14, 13, 12
};

void pascal near pillar_add(
	screen_x_t left, screen_y_t top, int duration, int patnum
);

int stone_kill_frame[STONE_COUNT] = { 0 };
extern uint8_t byte_2066E;
unsigned char angle_1EB28 = 0;
extern screen_x_t stones_pillar_left;
extern uint8_t stones_pillar_state;
extern unsigned char player_invincibility_time;

void pascal near sub_FFF8(screen_x_t center_x, screen_y_t center_y, int frame);
void pascal items_add(screen_x_t left, screen_y_t top, int type);

void stones_bg_render(void)
{
	register int i;

	for(i = 0; i < STONE_COUNT; i++) {
		if(stone_flag[i] < SF_REMOVED) {
			tiles_invalidate_rect(stone_left[i], stone_top[i], STONE_W, STONE_W);
			if(stone_flag[i] == SF_REMOVE) {
				stone_flag[i] = SF_REMOVED;
			}
		}
	}
}

void near stones_116EC(void)
{
	register int i;
	register vram_y_t vram_top;

	for(i = 0; i < STONE_COUNT; i++) {
		if(stone_flag[i] < SF_KILL_ANIM) {
			vram_top = stone_top[i];
			vram_top += scroll_line;
			if(vram_top >= RES_Y) {
				vram_top -= RES_Y;
			}
			if(stone_unknown[i]) {
				snd_se_play(4);
				super_roll_put_1plane(
					stone_left[i],
					vram_top,
					stone_patnum[i],
					0,
					super_plane(V_WHITE)
				);
				stone_unknown[i] = 0;
			} else {
				super_roll_put(stone_left[i], vram_top, stone_patnum[i]);
			}
		}
	}
}

int pascal near stones_11766(int stone)
{
	int patnum;
	int frames;
	vram_y_t vram_top;

	frames = 64;
	sparks_add(
		(stone_left[stone] + 16), (stone_top[stone] + 12), (5 << 4), 2, false
	);
	patnum = 10;
	vram_top = stone_top[stone];
	vram_top += scroll_line;
	if(vram_top >= RES_Y) {
		vram_top -= RES_Y;
	}
	if(stone == STONE_NORTH) {
		frames += 32;
		sub_FFF8(
			(stone_left[STONE_NORTH] + 8),
			(stone_top[STONE_NORTH] + 8),
			stone_kill_frame[STONE_NORTH]
		);
		if(stone_kill_frame[STONE_NORTH] >= 24) {
			sub_FFF8(
				(stone_left[STONE_NORTH] + 8),
				(stone_top[STONE_NORTH] + 8),
				(stone_kill_frame[STONE_NORTH] - 24)
			);
		}
		if(stone_kill_frame[STONE_NORTH] < 32) {
			super_roll_put(
				stone_left[STONE_NORTH], vram_top, stone_patnum[STONE_NORTH]
			);
			stone_kill_frame[STONE_NORTH]++;
			return 0;
		}
		if(stone_kill_frame[STONE_NORTH] == 56) {
			angle_1E510 = -0x20;
		}
		patnum += ((stone_kill_frame[STONE_NORTH] - 32) / 8);
	} else {
		patnum += (stone_kill_frame[stone] / 8);
	}
	stone_kill_frame[stone]++;
	if(stone_kill_frame[stone] >= frames) {
		stone_kill_frame[stone] = 0;
		stone_flag[stone] = SF_REMOVE;
		return 1;
	}
	super_put(stone_left[stone], vram_top, patnum);
	return 0;
}

void near stones_11877(void)
{
	register int i;
	register int damage;

	stones_pattern_frame++;
	for(i = 0; i < STONE_COUNT; i++) {
		if(stone_flag[i] > SF_ACTIVE) {
			continue;
		}
		if(
			!(damage = shots_hittest(stone_left[i], stone_top[i], STONE_W, 40))
			|| (stone_flag[i] != SF_ACTIVE)
		) {
			continue;
		}
		stone_unknown[i] = 1;
		stone_damage[i] += damage;
		if(
			(
				(i == STONE_NORTH) ? 930 : (i <= STONE_INNER_EAST) ? 140 : 120
			) <= stone_damage[i]
		) {
			stone_flag[i] = SF_KILL_ANIM;
			score_delta += 30000;
			if(i <= STONE_OUTER_EAST) {
				items_add(
					(stone_left[i] + 8),
					(stone_top[i] + 12),
					((i == 0) ? static_cast<int>(IT_BOMB) : (i <= 2) ? static_cast<int>(IT_BIGPOWER) : IT_BOMB)
				);
				snd_se_play(2);
			} else {
				player_invincibility_time = BOSS_DEFEAT_INVINCIBILITY_FRAMES;
			}
		}
	}
	for(i = 0; i < STONE_COUNT; i++) {
		if(stone_flag[i] == SF_ACTIVE) {
			shot_target_left = (stone_left[i] + 8);
			shot_target_top = (stone_top[i] + 8);
			break;
		}
	}
	if(player_topleft.y < (PLAYFIELD_TOP + 80)) {
		bullets_add_pellet(
			stones_bullet_left,
			stones_bullet_top,
			(randring2_next8_and(0x0F) - 7),
			BG_1_AIMED,
			((4 << 4) + 13)
		);
	}
}

void near stones_11997(void)
{
	if(rank != RANK_EASY) {
		byte_2066E = BG_5_SPREAD_WIDE_AIMED;
		byte_2066F = 0;
		group_20670 = BG_8_RING;
		byte_20671 = BG_4_RING;
	} else {
		byte_2066E = BG_3_SPREAD_WIDE_AIMED;
		byte_2066F = 2;
		group_20670 = BG_4_RING;
		byte_20671 = BG_2_RING;
	}
}

void near stones_119CD(void)
{
	register int i;

	if(boss_phase_frame < 30) {
		return;
	}
	if(boss_phase_frame < 180) {
		if((boss_phase_frame % 24) == 0) {
			for(i = 0; i < 2; i++) {
				if(stone_flag[i] == SF_ACTIVE) {
					bullets_add_pellet(
						(stone_left[i] + 12),
						(stone_top[i] + 8),
						0x00,
						byte_2066E,
						((2 << 4) + 6)
					);
				}
			}
		}
	} else if(boss_phase_frame >= 200) {
		if(boss_phase_frame < 310) {
			if((boss_phase_frame & 0xF) == 0) {
				for(i = 0; i < STONE_OUTER_WEST; i++) {
					if(stone_flag[i] == SF_ACTIVE) {
						bullets_add_16x16(
							(stone_left[i] + 12),
							(stone_top[i] + 8),
							0x00,
							BG_1_AIMED,
							PAT_BULLET16_OUTLINED_BALL_RED,
							(5 << 4)
						);
					}
				}
			}
		} else if(boss_phase_frame > 350) {
			boss_phase_frame = 0;
		}
	}
}

#define stones_outer_spread(i, stone) { \
	for(stone = STONE_OUTER_WEST; stone < STONE_NORTH; stone++) { \
		if(stone_flag[stone] == SF_ACTIVE) { \
			for(i = 0; i < 6; i++) { \
				bullets_add_16x16( \
					(stone_left[stone] + 12), \
					(stone_top[stone] + 8), \
					(i * 0x2A), \
					BSM_DECELERATE_THEN_TURN_AIMED, \
					PAT_BULLET16_OUTLINED_BALL_GREEN, \
					((3 << 4) + 8) \
				); \
			} \
		} \
	} \
}

void pascal near stones_11A87(int stones_removed)
{
	register int i;
	register int stone;

	if(boss_phase_frame < 30) {
		return;
	}
	if(boss_phase_frame < 200) {
		if((boss_phase_frame % (110 - (stones_removed * 20))) == 0) {
			stones_outer_spread(i, stone);
		}
	} else {
		if((boss_phase_frame % (90 - (stones_removed * 20))) == 0) {
			stones_outer_spread(i, stone);
		}
	}
}

void near stones_11B5D(void)
{
	if(boss_phase_frame < 50) {
		return;
	}
	if(boss_phase_frame == 50) {
		pillar_frames_new = 36;
		pillar_add(48, 96, 16, 0x67);
		pillar_add(192, 96, 16, 0x67);
		pillar_add(240, 96, 16, 0x67);
		pillar_add(384, 96, 16, 0x67);
	} else if(boss_phase_frame == 80) {
		pillar_add(80, 96, 16, 0x67);
		pillar_add(160, 96, 16, 0x67);
		pillar_add(272, 96, 16, 0x67);
		pillar_add(352, 96, 16, 0x67);
		boss_phase_frame = 0;
	}
}

void near stones_11BFE(void)
{
	if((boss_phase_frame & 3) == 0) {
		bullets_add_pellet(
			stones_bullet_left,
			stones_bullet_top,
			angle_1EB28,
			BG_2_SPREAD_HORIZONTALLY_SYMMETRIC,
			((4 << 4) + 6)
		);
		angle_1EB28 += 8;
		if(angle_1EB28 > 0x82) {
			boss_phase_frame = 0;
			angle_1EB28 = 0;
		}
	}
}

void near stones_11C37(void)
{
	screen_x_t left;

	if(boss_phase_frame < 24) {
		return;
	}
	if((boss_phase_frame % (28 - (rank * 3))) == 0) {
		pillar_frames_new = 30;
		left = ((player_topleft.x + (PLAYER_W / 2)) & 0xFFF0);
		pillar_add(left, 96, 16, 0x67);
	}
	if(boss_phase_frame >= 140) {
		boss_phase_frame = 0;
	}
}

void near stones_11C8A(void)
{
	unsigned char angle_1;
	unsigned char angle_2;

	if(boss_phase_frame < 40) {
		if((boss_phase_frame % 6) == 0) {
			angle_1 = iatan2(
				(player_topleft.y - 32),
				(player_topleft.x - stone_left[STONE_NORTH])
			);
			angle_2 = (randring2_next8_and(0x1F) + 0x10);
			bullets_add_pellet(
				stones_bullet_left,
				stones_bullet_top,
				(angle_1 + angle_2),
				BG_1,
				(5 << 4)
			);
			bullets_add_pellet(
				stones_bullet_left,
				stones_bullet_top,
				(angle_1 - angle_2),
				BG_1,
				(5 << 4)
			);
		}
	} else if(boss_phase_frame < 102) {
		if((boss_phase_frame % 6) == 0) {
			angle_1 = iatan2(
				(player_topleft.y - 32),
				(player_topleft.x - stone_left[STONE_NORTH])
			);
			angle_2 = (0x5F - boss_phase_frame);
			bullets_add_pellet(
				stones_bullet_left,
				stones_bullet_top,
				(angle_1 + angle_2),
				BG_1,
				(5 << 4)
			);
			bullets_add_pellet(
				stones_bullet_left,
				stones_bullet_top,
				(angle_1 - angle_2),
				BG_1,
				(5 << 4)
			);
		}
	} else {
		boss_phase_frame = 0;
	}
}

void near stones_11D30(void)
{
	if(boss_phase_frame < 16) {
		return;
	}
	if(boss_phase_frame == 16) {
		stones_pillar_left = 32;
		stones_pillar_state = 0;
	}
	if((stones_pillar_state == 0) && ((boss_phase_frame & 7) == 0)) {
		pillar_frames_new = 16;
		pillar_add(stones_pillar_left, 96, 3, 0x6F);
		pillar_add((432 - stones_pillar_left), 96, 3, 0x6F);
		stones_pillar_left += 16;
		if(stones_pillar_left > 176) {
			stones_pillar_state = 1;
			stones_pillar_left += 16;
		}
	}
	if(stones_pillar_state > 36) {
		if((boss_phase_frame & 7) == 0) {
			pillar_add(stones_pillar_left, 96, 1, 0x6F);
			pillar_add((432 - stones_pillar_left), 96, 1, 0x6F);
			stones_pillar_left -= 16;
			if(stones_pillar_left < 64) {
				stones_pillar_state = 0;
				boss_phase_frame = 0;
				stones_pillar_left = 32;
			}
		}
	} else if(stones_pillar_state != 0) {
		stones_pillar_state++;
	}
}

void near stones_11DF6(void)
{
	register int i;

	if((boss_phase_frame & 0xF) == 0) {
		for(i = byte_2066F; i < STONE_NORTH; i++) {
			bullets_add_16x16(
				stone_left[i],
				stone_top[i],
				randring2_next8(),
				BSM_GRAVITY,
				PAT_BULLET16_OUTLINED_BALL_GREEN,
				(2 << 4)
			);
		}
	}
	if(boss_phase_frame > 110) {
		boss_phase_frame = 0;
	}
}

void near stones_11E40(void)
{
	if((boss_phase_frame & 3) == 0) {
		bullets_add_pellet(
			stones_bullet_left,
			stones_bullet_top,
			randring2_next8(),
			byte_20671,
			((boss_phase_frame >> 1) + ((1 << 4) + 14))
		);
	}
	if(boss_phase_frame > 130) {
		boss_phase_frame = 0;
	}
}

void near stones_11E76(void)
{
	register int i;
	register int column;

	if(boss_phase_frame < 16) {
		return;
	}
	if(boss_phase_frame < 80) {
		if((boss_phase_frame % 36) == 0) {
			pillar_frames_new = 30;
			column = ((randring2_next8() % 24) + 2);
			pillar_add((column * 16), 96, 12, 0x6F);
		}
		if(((boss_phase_frame % 40) == 0) && (rank != RANK_EASY)) {
			bullets_add_pellet(
				stones_bullet_left,
				stones_bullet_top,
				0x00,
				BG_1_AIMED,
				((4 << 4) + 6)
			);
		}
	} else if(boss_phase_frame < 120) {
		if(boss_phase_frame == 100) {
			pillar_frames_new = 30;
			for(i = 0; i < 3; i++) {
				column = ((randring2_next8() % 24) + 2);
				pillar_add((column * 16), 96, 12, 0x6F);
			}
		}
	} else {
		boss_phase_frame = 0;
	}
}

void near stones_11F2F(void)
{
	register int i;

	if(boss_phase_frame < 16) {
		return;
	}
	if(boss_phase_frame == 16) {
		angle_22FAF = 0x29;
	}
	if((boss_phase_frame % 30) == 0) {
		for(i = 0; i < STONE_NORTH; i++) {
			bullets_add_16x16(
				stone_left[i],
				stone_top[i],
				angle_22FAF,
				BG_1_AIMED,
				PAT_BULLET16_OUTLINED_BALL_GREEN,
				((3 << 4) + 12)
			);
			bullets_add_16x16(
				stone_left[i],
				stone_top[i],
				-angle_22FAF,
				BG_1_AIMED,
				PAT_BULLET16_OUTLINED_BALL_GREEN,
				((3 << 4) + 12)
			);
		}
		angle_22FAF -= 3;
	}
	if(angle_22FAF >= 0xC8) {
		boss_phase_frame = 0;
	}
}

void near stones_11FB5(void)
{
	if(boss_phase_frame == 2) {
		angle_22FB0 = 0x3E;
	}
	if((boss_phase_frame & 7) == 0) {
		bullets_add_pellet(
			stones_bullet_left,
			stones_bullet_top,
			(0x00 - angle_22FB0),
			group_20670,
			(4 << 4)
		);
		bullets_add_pellet(
			stones_bullet_left,
			stones_bullet_top,
			angle_22FB0,
			group_20670,
			(4 << 4)
		);
		angle_22FB0 -= 3;
		if(angle_22FB0 > 0xC8) {
			boss_phase_frame = 0;
		}
	}
}

void near stones_1200F(void)
{
	register int i;

	if(boss_phase_frame < 60) {
		return;
	}
	if((boss_phase_frame % 64) == 0) {
		for(i = 0; i < STONE_COUNT; i++) {
			angle_22FB1[i] = iatan2(
				(player_topleft.y - 32), (player_topleft.x - stone_left[i])
			);
		}
	}
	if((boss_phase_frame & 3) == 0) {
		if(boss_phase_frame < 384) {
			if(((boss_phase_frame % 64) >> 2) <= 8) {
				for(i = 0; i < STONE_COUNT; i++) {
					bullets_add_pellet(
						(stone_left[i] + 12),
						(stone_top[i] + 12),
						angle_22FB1[i],
						BG_1,
						(7 << 4)
					);
				}
				snd_se_play(3);
			}
		} else {
			bullets_set_stack_multiplier(0);
			for(i = 0; i < STONE_COUNT; i++) {
				bullets_add_16x16(
					(stone_left[i] + 8),
					(stone_top[i] + 8),
					angle_22FB1[STONE_NORTH],
					BG_4_RING,
					PAT_BULLET16_BALL,
					(7 << 4)
				);
			}
			snd_se_play(3);
		}
	}
	if(boss_phase_frame >= 447) {
		boss_phase_frame = 0;
		bullets_set_stack_multiplier(1);
	}
}

int pascal near stones_120F7(int stone)
{
	if((stone == STONE_INNER_WEST) || (stone == STONE_INNER_EAST)) {
		stone_patnum[stone]++;
		if(stone_patnum[stone] >= 155) {
			stone_flag[stone] = SF_ACTIVE;
			return 1;
		}
	} else if((stone == STONE_OUTER_WEST) || (stone == STONE_OUTER_EAST)) {
		if(stone_patnum[stone] < 152) {
			stone_patnum[stone]++;
		}
		if(stone_patnum[stone] == 152) {
			stone_patnum[stone] = 156;
		} else {
			stone_patnum[stone]++;
		}
		if(stone_patnum[stone] >= 159) {
			stone_flag[stone] = SF_ACTIVE;
			return 1;
		}
	} else if(stone == STONE_NORTH) {
		if(stone_patnum[STONE_NORTH] < 152) {
			stone_patnum[STONE_NORTH]++;
		}
		if(stone_patnum[STONE_NORTH] == 152) {
			stone_patnum[STONE_NORTH] = 160;
		} else {
			stone_patnum[STONE_NORTH]++;
		}
		if(stone_patnum[STONE_NORTH] >= 163) {
			stone_flag[STONE_NORTH] = SF_ACTIVE;
			return 1;
		}
	}
	return 0;
}

int near stones_121BA(void)
{
	if(stone_patnum[STONE_NORTH] >= 160) {
		stone_patnum[STONE_NORTH]--;
	}
	if(stone_patnum[STONE_NORTH] == 159) {
		stone_flag[STONE_NORTH] = SF_DORMANT;
		stone_patnum[STONE_NORTH] = 151;
	} else {
		stone_patnum[STONE_NORTH]--;
	}
	if(stone_patnum[STONE_NORTH] <= 148) {
		return 1;
	}
	return 0;
}

void pascal near stones_ring_step(uint8_t flag)
{
	if(boss_phase_frame & 1) {
		stones_ring_tile_flag[STONES_RING_ORDER_LEFT[stones_ring_i]] = flag;
		stones_ring_tile_flag[STONES_RING_ORDER_RIGHT[stones_ring_i]] = flag;
		stones_ring_i++;
		if(stones_ring_i >= (TILES_X / 2)) {
			stones_ring_phase++;
			stones_ring_i = 0;
		}
	}
}

void near stones_ring_put(void)
{
	screen_x_t left;
	register uint8_t near *p = stones_ring_tile_flag;
	register int i;

	egc_start_copy_noframe();
	for((i = 0, left = PLAYFIELD_LEFT); i < TILES_X; (i++, left += TILE_W, p++)) {
		if(*p == 1) {
			tile_ring_set_and_put_both_8(left, stones_y_22D9C, stones_ring_tile_1);
			*p = 0;
		} else if(*p == 2) {
			tile_ring_set_and_put_both_8(left, stones_y_22D9C, stones_ring_tile_2);
			*p = 0;
		} else if(stones_ring_tile_flag[i] == 3) {
			tile_ring_set_and_put_both_8(left, stones_y_22D9C, stones_ring_tile_3);
			*p = 0;
			stones_ring_done++;
		}
	}
	egc_off();
}

#define stones_ring_animate(tile_1, tile_2, tile_3) { \
	if(boss_phase_frame < 49) { \
		return 0; \
	} \
	if(boss_phase_frame == 49) { \
		for(i = 0; i < TILES_X; i++) { \
			stones_ring_tile_flag[i] = 0; \
		} \
		stones_ring_i = 0; \
		stones_ring_phase = 0; \
		stones_ring_done = 0; \
		stones_ring_tile_1 = tile_1; \
		stones_ring_tile_2 = tile_2; \
		stones_ring_tile_3 = tile_3; \
	} else if(stones_ring_phase == 0) { \
		stones_ring_step(1); \
	} else if(stones_ring_phase == 1) { \
		stones_ring_step(2); \
	} else if(stones_ring_phase == 2) { \
		stones_ring_step(3); \
	} \
	stones_ring_put(); \
	if(stones_ring_done >= TILES_X) { \
		return 1; \
	} \
	return 0; \
}

int near stones_122B5(void)
{
	register int i;
	stones_ring_animate(41, 42, 43);
}

int near stones_1232F(void)
{
	register int i;
	stones_ring_animate(42, 41, 40);
}

int stones_update(void)
{
	register int i;
	register int removed = 0;

	boss_phase_frame++;
	stone_frames++;
	stones_11877();
	stones_116EC();
	for(i = 0; i < STONE_COUNT; i++) {
		if(stone_flag[i] == SF_KILL_ANIM) {
			stones_11766(i);
		} else if(stone_flag[i] == SF_REMOVED) {
			removed++;
		}
	}
	if(removed >= STONE_COUNT) {
		return SP_CLEAR;
	}
	if(stones_phase == 0) {
		if(
			(boss_phase_frame == 50) ||
			(boss_phase_frame == 52) ||
			(boss_phase_frame == 54) ||
			(boss_phase_frame == 58) ||
			(boss_phase_frame == 60) ||
			(boss_phase_frame == 62)
		) {
			stone_patnum[STONE_INNER_WEST]++;
			stone_patnum[STONE_INNER_EAST]++;
		} else if(boss_phase_frame == 64) {
			stone_patnum[STONE_INNER_WEST]++;
			stone_patnum[STONE_INNER_EAST]++;
			boss_phase_frame = 0;
			stone_flag[STONE_INNER_WEST] = SF_ACTIVE;
			stone_flag[STONE_INNER_EAST] = SF_ACTIVE;
			stones_phase = 1;
			stones_pattern_frame = 0;
		}
	} else if(stones_phase == 1) {
		stones_119CD();
		if(stone_frames > 1300) {
			if(stone_flag[STONE_INNER_WEST] == SF_ACTIVE) {
				stone_flag[STONE_INNER_WEST] = SF_KILL_ANIM;
			}
			if(stone_flag[STONE_INNER_EAST] == SF_ACTIVE) {
				stone_flag[STONE_INNER_EAST] = SF_KILL_ANIM;
			}
		}
		if((removed >= 2) && ((boss_phase_frame % 3) == 0)) {
			stones_120F7(STONE_OUTER_WEST);
			if(stones_120F7(STONE_OUTER_EAST)) {
				stones_phase = 2;
				boss_phase_frame = 0;
				stones_pattern_frame = 0;
			}
			stone_frames = 0;
		}
	} else if(stones_phase == 2) {
		if(stone_frames > 1300) {
			if(stone_flag[STONE_OUTER_WEST] == SF_ACTIVE) {
				stone_flag[STONE_OUTER_WEST] = SF_KILL_ANIM;
			}
			if(stone_flag[STONE_OUTER_EAST] == SF_ACTIVE) {
				stone_flag[STONE_OUTER_EAST] = SF_KILL_ANIM;
			}
		}
		if(removed >= 4) {
			stones_phase = 3;
			boss_phase_frame = 0;
			stones_pattern = 0;
			stones_pattern_frame = 0;
			shot_target_left = (stone_left[STONE_NORTH] + 8);
			shot_target_top = 32;
		} else {
			stones_11A87(removed);
		}
	} else if(stones_phase == 3) {
		if(
			stones_122B5() &&
			((boss_phase_frame % 3) == 0) &&
			stones_120F7(STONE_NORTH)
		) {
			stones_phase = 4;
			boss_phase_frame = 0;
			stones_pattern_frame = 0;
			stone_frames = 0;
		}
	} else if(stones_phase == 4) {
		if(stones_pattern == 0) {
			stones_11B5D();
		} else if(stones_pattern == 1) {
			stones_11BFE();
		} else if(stones_pattern == 2) {
			stones_11C37();
		} else if(stones_pattern == 3) {
			stones_11DF6();
		} else if(stones_pattern == 4) {
			stones_11E40();
		} else if(stones_pattern == 5) {
			stones_11E76();
		}
		if(boss_phase_frame == 0) {
			stones_pattern++;
			stones_pattern_frame = 0;
			if(stones_pattern > 5) {
				stones_phase = 5;
				stones_pattern = 0;
			}
		}
		if(stone_damage[STONE_NORTH] >= 500) {
			stone_damage[STONE_NORTH] = 500;
		}
	} else if(stones_phase == 5) {
		if(((boss_phase_frame % 3) == 0) && (stones_pattern == 0)) {
			stones_pattern = stones_121BA();
		}
		if(stones_1232F()) {
			stones_phase = 6;
			boss_phase_frame = 0;
			stones_pattern_frame = 0;
			stone_frames = 0;
			stones_pattern = 0;
		}
	} else if(stones_phase == 6) {
		if(stones_pattern == 0) {
			stones_11FB5();
		} else if(stones_pattern == 1) {
			stones_11F2F();
		} else if(stones_pattern == 2) {
			stones_11FB5();
		} else if(stones_pattern == 3) {
			stones_1200F();
		}
		if(boss_phase_frame == 0) {
			stones_pattern++;
			stones_pattern_frame = 0;
			if(stones_pattern > 3) {
				stones_phase = 7;
			}
		}
	} else if(stones_phase == 7) {
		if(
			stones_122B5() &&
			((boss_phase_frame % 3) == 0) &&
			stones_120F7(STONE_NORTH)
		) {
			stones_phase = 8;
			boss_phase_frame = 0;
			stones_pattern_frame = 0;
			stone_frames = 0;
			stones_pattern = 0;
		}
	} else if(stones_phase == 8) {
		if(stones_pattern == 0) {
			stones_11D30();
		} else if(stones_pattern == 1) {
			stones_11BFE();
		} else if(stones_pattern == 2) {
			stones_11C8A();
		} else if(stones_pattern == 3) {
			stones_11DF6();
		} else if(stones_pattern == 4) {
			stones_11E40();
		} else if(stones_pattern == 5) {
			stones_11E76();
		}
		if(boss_phase_frame == 0) {
			stones_pattern++;
			stones_pattern_frame = 0;
			if(stones_pattern > 5) {
				stones_pattern = 1;
			}
			if(stone_frames > 2000) {
				stone_flag[STONE_NORTH] = SF_KILL_ANIM;
			}
		}
	}
	return SP_BOSS;
}

void stones_end(void)
{
	dialog_pre();
	dialog_script_generic_part_animate(DS_POSTBOSS);
	stage_clear_bonus_animate();
	overlay_stage_leave_animate();
	stage_id++;
}

void near stones_12778(void);

void stones_init(void)
{
	dialog_pre();
	dialog_script_generic_part_animate(DS_PREBOSS);
	dialog_post();
	sub_13ABB("boss2.m");
	stones_12778();
	sub_129FC();
}

void near stones_12778(void)
{
	register int i;

	boss_phase_frame = 0;
	for(i = 0; i < STONE_COUNT; i++) {
		stone_flag[i] = SF_DORMANT;
		stone_damage[i] = 0;
		stone_patnum[i] = 148;
		stone_unknown[i] = 0;
	}
	stone_left[STONE_INNER_WEST] = (PLAYFIELD_LEFT + 16 + (1 * 80));
	stone_top[STONE_INNER_WEST] = (PLAYFIELD_TOP + 16);
	stone_left[STONE_INNER_EAST] = (PLAYFIELD_LEFT + 16 + (3 * 80));
	stone_top[STONE_INNER_EAST] = (PLAYFIELD_TOP + 16);
	stone_left[STONE_OUTER_WEST] = (PLAYFIELD_LEFT + 16 + (0 * 80));
	stone_top[STONE_OUTER_WEST] = (PLAYFIELD_TOP + 32);
	stone_left[STONE_OUTER_EAST] = (PLAYFIELD_LEFT + 16 + (4 * 80));
	stone_top[STONE_OUTER_EAST] = (PLAYFIELD_TOP + 32);
	stone_left[STONE_NORTH] = (PLAYFIELD_LEFT + 16 + (2 * 80));
	stone_top[STONE_NORTH] = (PLAYFIELD_TOP + 16);
	stones_bullet_top = (PLAYFIELD_TOP + 24);
	stones_bullet_left = (PLAYFIELD_LEFT + (PLAYFIELD_W / 2) - (PELLET_W / 2));
	sub_17A55();
	pillar_frames_new = 12;
	stones_pattern_frame = 0;
	stone_frames = 0;
	stones_phase = 0;
	stones_pattern = 0;
	angle_1E510 = 0x20;
	stones_y_22D9C = 96;
	stones_y_22D9C += scroll_line;
	if(stones_y_22D9C >= RES_Y) {
		stones_y_22D9C -= RES_Y;
	}
	stones_11997();
}
