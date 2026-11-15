/// Stage 3 background effect and midboss
/// --------------------------------------

extern "C" void MASTER_RET graph_scroll(
	unsigned line1, unsigned adr1, unsigned adr2
);

extern unsigned int word_20686;
extern pixel_t stage3_bg_radius;
extern screen_x_t stage3_bg_center_x;
extern screen_y_t stage3_bg_center_y;
extern unsigned char stage3_bg_angle;
extern screen_point_t stage3_bg_dots[PAGE_COUNT][64];

void sub_10E4F(void)
{
	register int i;
	for(i = 0; i < 64; i++) {
		tiles_invalidate_rect(
			stage3_bg_dots[page_back][i].x, stage3_bg_dots[page_back][i].y, 1, 2
		);
	}
}

void sub_10E95(void)
{
	screen_x_t left;
	unsigned char angle;
	register screen_y_t top;
	register int i;

	word_20686++;
	if(word_20686 < 168) {
		return;
	}
	if(word_20686 == 168) {
		snd_se_play(14);
	} else if(word_20686 < 200) {
		if(word_20686 <= 180) {
			PaletteTone = (((word_20686 & 1) * 70) + 100);
			palette_show();
		}
		i = (word_20686 & 1);
		graph_scroll((RES_Y - scroll_line), ((scroll_line * 40) + i), i);
		slowdown_factor = 3;
		stage3_bg_center_x = 224;
		stage3_bg_center_y = 200;
		stage3_bg_radius = 0;
		stage3_bg_angle = 0;
	} else if(word_20686 < 400) {
		if(word_20686 == 200) {
			slowdown_factor = 1;
		}
		Palettes[0].c.r = (200 - (word_20686 >> 1));
		Palettes[0].c.g = 0;
		Palettes[0].c.b = 0;
		stage3_bg_radius = (word_20686 - 200);
		stage3_bg_angle++;
	} else if(word_20686 < 600) {
		Palettes[0].c.r = 0;
		Palettes[0].c.g = (44 - (word_20686 >> 1));
		Palettes[0].c.b = 0;
		stage3_bg_radius = (word_20686 - 400);
		stage3_bg_angle--;
	} else if(word_20686 < 800) {
		Palettes[0].c.r = 0;
		Palettes[0].c.g = 0;
		Palettes[0].c.b = (144 - (word_20686 >> 1));
		stage3_bg_radius = (word_20686 - 600);
		stage3_bg_angle++;
	} else if(word_20686 < 1000) {
		Palettes[0].c.r = (244 - (word_20686 >> 1));
		Palettes[0].c.g = 0;
		Palettes[0].c.b = (244 - (word_20686 >> 1));
		stage3_bg_radius = (word_20686 - 800);
		stage3_bg_angle--;
	} else {
		word_20686 = 200;
	}
	palette_show();
	egc_off();
	grcg_setcolor(GC_RMW, 7);
	for((i = 0, angle = stage3_bg_angle); i < 64; (i++, angle += 4)) {
		left = polar_x_fast(stage3_bg_center_x, stage3_bg_radius, angle);
		top = polar_y_fast(stage3_bg_center_y, stage3_bg_radius, angle);
		stage3_bg_dots[page_back][i].x = left;
		stage3_bg_dots[page_back][i].y = top;
		top += scroll_line;
		if(top >= RES_Y) {
			top -= RES_Y;
		}
		grcg_pset(left, top);
	}
	grcg_off();
}

// State
// -----

int16_t midboss3_kill_frame[MIDBOSS3_COUNT] = { 0 };

// Same as for regular bosses.
// ZUN bloat: We only need two for each of these, not 5.
extern screen_x_t midboss3_left_on_page[STONE_COUNT][PAGE_COUNT];
extern screen_y_t midboss3_top_on_page[STONE_COUNT][PAGE_COUNT];
extern screen_x_t* midboss3_left_on_back_page[STONE_COUNT];
extern screen_y_t* midboss3_top_on_back_page[STONE_COUNT];

extern unsigned char midboss3_angle[MIDBOSS3_COUNT];
extern uint8_t midboss3_rounds;
// -----

int midboss3_invalidate(void)
{
	register int i;
	register int removed = 0;

	for(i = 0; i < MIDBOSS3_COUNT; i++) {
		if(midboss3_flag[i] == M3F_REMOVED) {
			removed++;
			continue;
		}
		midboss3_left_on_back_page[i] = &midboss3_left_on_page[i][page_back];
		midboss3_top_on_back_page[i] = &midboss3_top_on_page[i][page_back];
		tiles_invalidate_rect(
			*midboss3_left_on_back_page[i], *midboss3_top_on_back_page[i], 64, 32
		);
		*midboss3_left_on_back_page[i] = midboss3_left_on_page[i][page_front];
		*midboss3_top_on_back_page[i] = midboss3_top_on_page[i][page_front];
	}
	if(removed == MIDBOSS3_COUNT) {
		return false;
	}
	return true;
}

void pascal near midboss3_hit_put(int i)
{
	register vram_y_t vram_top;

	snd_se_play(4);
	vram_top = *midboss3_top_on_back_page[i];
	vram_top += scroll_line;
	if(vram_top >= RES_Y) {
		vram_top -= RES_Y;
	}
	super_roll_put_1plane(
		*midboss3_left_on_back_page[i],
		vram_top,
		stone_patnum[STONE_INNER_WEST],
		0,
		super_plane(V_WHITE)
	);
}

void pascal near midboss3_put(int i)
{
	register vram_y_t vram_top;

	vram_top = *midboss3_top_on_back_page[i];
	vram_top += scroll_line;
	if(vram_top >= RES_Y) {
		vram_top -= RES_Y;
	}
	super_roll_put(
		*midboss3_left_on_back_page[i], vram_top, stone_patnum[STONE_INNER_WEST]
	);
}

void pascal near midboss3_kill_animate(int i)
{
	int patnum;
	vram_y_t vram_top;

	sparks_add(
		(*midboss3_left_on_back_page[i] + 32),
		(*midboss3_top_on_back_page[i] + 16),
		((3 << 4) + 12),
		1,
		false
	);
	patnum = 10;
	patnum += (midboss3_kill_frame[i] / 6);
	midboss3_kill_frame[i]++;
	if(midboss3_kill_frame[i] >= 48) {
		sparks_add(
			(*midboss3_left_on_back_page[i] + 32),
			(*midboss3_top_on_back_page[i] + 16),
			(8 << 4),
			32,
			true
		);
		midboss3_kill_frame[i] = 0;
		midboss3_flag[i] = M3F_REMOVED;
		shot_target_left = -1;
		shot_target_top = -1;
	} else {
		vram_top = *midboss3_top_on_back_page[i];
		vram_top += scroll_line;
		if(vram_top >= RES_Y) {
			vram_top -= RES_Y;
		}
		super_roll_put(*midboss3_left_on_back_page[i], vram_top, patnum);
		super_roll_put((*midboss3_left_on_back_page[i] + 32), vram_top, patnum);
	}
}

void pascal near midboss3_fire(int i)
{
	int velocity;
	uint8_t group;
	screen_y_t top;

	velocity = ((i == 0) ? 8 : -8);
	top = (*midboss3_top_on_back_page[i] + 12);
	if(boss_phase_frame < 152) {
		if(boss_phase_frame == 128) {
			midboss3_angle[0] = 0x00;
			midboss3_angle[1] = 0x80;
		}
		if((boss_phase_frame & 3) == 0) {
			midboss3_angle[i] += velocity;
			bullets_add_pellet(
				(*midboss3_left_on_back_page[i] + 28),
				top,
				midboss3_angle[i],
				BG_1,
				((2 << 4) + 8)
			);
		}
	} else if(boss_phase_frame < 192) {
		*midboss3_left_on_back_page[i] += velocity;
		if((boss_phase_frame & 7) == 0) {
			midboss3_angle[i] += (velocity * 2);
			if(rank != RANK_EASY) {
				group = BG_4_SPREAD_MEDIUM;
			} else {
				group = BG_2_SPREAD_MEDIUM;
			}
			bullets_add_pellet(
				(*midboss3_left_on_back_page[i] + 28),
				top,
				(0x80 - midboss3_angle[i]),
				group,
				((2 << 4) + 8)
			);
		}
	} else if(boss_phase_frame < 352) {
		*midboss3_top_on_back_page[i] += 2;
		if(*midboss3_top_on_back_page[i] >= RES_Y) {
			*midboss3_top_on_back_page[i] -= RES_Y;
		}
	} else if(boss_phase_frame < 376) {
		if((boss_phase_frame & 3) == 0) {
			midboss3_angle[i] -= velocity;
			bullets_add_pellet(
				(*midboss3_left_on_back_page[i] + 28),
				top,
				midboss3_angle[i],
				BG_1,
				((3 << 4) + 2)
			);
		}
	} else if(boss_phase_frame < 416) {
		*midboss3_left_on_back_page[i] -= velocity;
		if((boss_phase_frame & 3) == 0) {
			midboss3_angle[i] -= velocity;
			bullets_add_pellet(
				(*midboss3_left_on_back_page[i] + 28),
				top,
				midboss3_angle[i],
				BG_1,
				((3 << 4) + 2)
			);
		}
	} else if(boss_phase_frame < 576) {
		*midboss3_top_on_back_page[i] -= 2;
		if(*midboss3_top_on_back_page[i] < 0) {
			*midboss3_top_on_back_page[i] += RES_Y;
		}
		if((boss_phase_frame % 60) == 0) {
			bullets_add_pellet(
				(*midboss3_left_on_back_page[i] + 28),
				top,
				0x00,
				BG_1_AIMED,
				((2 << 4) + 8)
			);
		}
	} else {
		boss_phase_frame = 127; // Skip the scroll-in animation
	}
}

extern uint8_t scroll_speed_this_frame;

void midboss3_update_and_render(void)
{
	int removed = 0;
	register int i;
	register int damage;

	if(midboss3_flag[0] == M3F_ALIVE) {
		shot_target_left = (midboss3_left_on_page[0][page_back] + 24);
		shot_target_top = (midboss3_top_on_page[0][page_back] + 16);
	} else {
		shot_target_left = (midboss3_left_on_page[1][page_back] + 24);
		shot_target_top = (midboss3_top_on_page[1][page_back] + 16);
	}
	boss_phase_frame++;
	if(boss_phase_frame == 1) {
		midboss3_left_on_page[0][0] = PLAYFIELD_LEFT;
		midboss3_left_on_page[0][1] = PLAYFIELD_LEFT;
		midboss3_top_on_page[0][0] = -16;
		midboss3_top_on_page[0][1] = -16;
		midboss3_damage[0] = 0;
		midboss3_left_on_page[1][0] = (PLAYFIELD_RIGHT - 64);
		midboss3_left_on_page[1][1] = (PLAYFIELD_RIGHT - 64);
		midboss3_top_on_page[1][0] = midboss3_top_on_page[0][0];
		midboss3_top_on_page[1][1] = midboss3_top_on_page[0][0];
		midboss3_damage[1] = 0;
		midboss3_rounds = 0;
	} else if(boss_phase_frame < 128) {
		stone_patnum[STONE_INNER_WEST] = ((scroll_line & 3) + 144);
		for(i = 0; i < MIDBOSS3_COUNT; i++) {
			*midboss3_top_on_back_page[i] += scroll_speed_this_frame;
			if((damage = shots_hittest(
				(*midboss3_left_on_back_page[i] + 12),
				*midboss3_top_on_back_page[i],
				40,
				30
			)) != 0) {
				midboss3_damage[i] += damage;
				midboss3_hit_put(i);
			} else {
				midboss3_put(i);
			}
		}
	} else {
		stone_patnum[STONE_INNER_WEST] = ((scroll_line & 3) + 144);
		if(boss_phase_frame == 128) {
			midboss3_rounds++;
		}
		for(i = 0; i < MIDBOSS3_COUNT; i++) {
			if(midboss3_flag[i] == M3F_KILL_ANIM) {
				midboss3_kill_animate(i);
			} else if(midboss3_flag[i] == M3F_ALIVE) {
				midboss3_fire(i);
				if((damage = shots_hittest(
					(*midboss3_left_on_back_page[i] + 12),
					*midboss3_top_on_back_page[i],
					40,
					30
				)) != 0) {
					midboss3_damage[i] += damage;
					if(midboss3_damage[i] <= 280) {
						midboss3_hit_put(i);
					} else {
						snd_se_play(2);
						midboss3_flag[i] = M3F_KILL_ANIM;
						score_delta += 20000;
					}
				} else {
					midboss3_put(i);
				}
				if(midboss3_rounds >= 4) {
					midboss3_flag[i] = M3F_KILL_ANIM;
				}
			} else if(midboss3_flag[i] == M3F_REMOVED) {
				removed++;
			}
		}
	}
}
