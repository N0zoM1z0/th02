#include "planar.h"

#define BOMB_PARTICLE_W 8
#define BOMB_PARTICLE_H 8
static const int BOMB_PARTICLE_CELS = 4;

typedef dots_t(BOMB_PARTICLE_W) bomb_particle_dots_t;

extern const dot_rect_t(BOMB_PARTICLE_W, BOMB_PARTICLE_H) sBOMB_PARTICLES[
	BOMB_PARTICLE_CELS + 1
];
#define sBOMB_CIRCLE sBOMB_PARTICLES[BOMB_PARTICLE_CELS].row
