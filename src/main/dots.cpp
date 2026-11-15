/* ReC98
 * -----
 * Moving dots, as fired by Marisa, drawn with a single 16-pixel VRAM word per
 * row. Placed in _TEXT between the master.lib code and the sparks.
 */

#pragma option -zC_TEXT -zPTEXT -G

#include "platform.h"
#include "x86real.h"
#include "pc98.h"
#include "planar.h"
#include "libs/master.lib/master.hpp"
#include "libs/master.lib/pc98_gfx.hpp"
#include "th02/hardware/pages.hpp"
#include "th02/main/circle.hpp"

enum dot_flag_t {
	DF_FREE = 0,
	DF_ALIVE = 1,
	DF_REMOVE = 2,
};

struct dot_t {
	uint8_t flag;
	uint8_t angle;

	// Subpixel coordinates, one point per VRAM page.
	point_t pos[PAGE_COUNT];

	uint8_t size;
	uint8_t speed;
};

static const int DOT_COUNT = 30;

// Uninitialized data, in the original order
#pragma option -a2
int16_t word_1FFF0; // Initial speed
int16_t word_1FFF2;
int16_t word_1FFF4;
int16_t word_1FFF6;
uint8_t byte_1FFF8;
uint8_t byte_1FFF9;
int8_t byte_1FFFA;
dot_t dots[DOT_COUNT];
screen_x_t circle_dot_left;
vram_y_t circle_dot_top;
uint8_t byte_20168;
uint8_t byte_20169;
#pragma option -a-

// Leftmost [size] bits set, for each dot size.
extern const uint8_t CIRCLE_DOT_PATTERNS[] = {
	0x00, 0x80, 0xC0, 0xE0, 0xF0, 0xF8
};

extern bool reduce_effects;

// ZUN landmine: Called with a near call from _TEXT into SHARED, which only
// works as long as vector2() stays within 64 KiB of _TEXT's frame.
void pascal near vector2(int far &x, int far &y, unsigned char angle, int length);

#define pos_at(p, page_offset) ( \
	*reinterpret_cast<point_t near *>( \
		reinterpret_cast<uint8_t near *>(p->pos) + page_offset \
	) \
)

void sub_3DDE(void)
{
	for(_BX = 0; static_cast<int>(_BX) < static_cast<int>(sizeof(dots)); _BX += sizeof(dot_t)) {
		*(reinterpret_cast<uint8_t near *>(dots) + _BX) = DF_FREE;
	}
	word_1FFF0 = 32;
	word_1FFF2 = 1;
	word_1FFF4 = 0;
	byte_1FFF8 = 15;
	byte_1FFF9 = 0;
	byte_1FFFA = -1;
	word_1FFF6 = 32;
}

void pascal sub_3E1C(screen_x_t left, screen_y_t top, unsigned char angle)
{
	register dot_t near *p;

	_CX = left;
	_BX = top;
	_CX <<= 4;
	_BX <<= 4;
	_AX = FP_OFF(dots);
	p = reinterpret_cast<dot_t near *>(_AX);
	for(_DX = 0; static_cast<int>(_DX) < DOT_COUNT; _DX++, p++) {
		if(p->flag == DF_FREE) {
			p->flag = DF_ALIVE;
			p->angle = angle;
			p->pos[0].x = _CX;
			p->pos[1].x = _CX;
			p->pos[0].y = _BX;
			p->pos[1].y = _BX;
			p->size = 1;
			p->speed = word_1FFF0;
			break;
		}
	}
}

#pragma codestring "\x90"

void pascal circle_dot_put(int size)
{
	register int h;
	unsigned int pattern;

	_ES = SEG_PLANE_B;
	h = size;
	_DI = vram_offset_shift_fast(circle_dot_left, circle_dot_top);
	_BX = circle_dot_left;
	_BX &= BYTE_MASK;
	pattern = CIRCLE_DOT_PATTERNS[h];
	_AX = ((pattern >> _BL) | (pattern << (16 - _BL)));
	while(h > 0) {
		_poke_(_ES, _DI, _AX);
		_DI += ROW_SIZE;
		h--;
	}
}

// Fills [h] rows of 16 pixels at ([circle_dot_left], [circle_dot_top]),
// with the GRCG already set and [_ES] pointing to VRAM.
void pascal near dot_unput(int h)
{
	_SI = h;
	_BX = vram_offset_shift_fast(circle_dot_left, circle_dot_top);
	while(static_cast<int>(_SI) > 0) {
		_poke_(_ES, _BX, 0xFFFF);
		_BX += ROW_SIZE;
		_SI--;
	}
}

#pragma codestring "\x90"

void sub_3EF4(void)
{
	register dot_t near *p;
	register int i;

	_ES = SEG_PLANE_B;
	grcg_setcolor(GC_RMW, byte_1FFF9);
	_AX = FP_OFF(dots);
	p = reinterpret_cast<dot_t near *>(_AX);
	for(i = 0; i < DOT_COUNT; i++, p++) {
		if(p->flag == DF_FREE) {
			continue;
		}
		_BX = (page_back * sizeof(point_t));
		circle_dot_left = (pos_at(p, _BX).x >> 4);
		circle_dot_top = (pos_at(p, _BX).y >> 4);
		if(!reduce_effects || (page_back == (i & 1))) {
			dot_unput(p->size);
		}
		if(p->flag == DF_REMOVE) {
			p->flag = DF_FREE;
			continue;
		}
		_BX = (page_front * sizeof(point_t));
		_AX = pos_at(p, _BX).x;
		_CX = pos_at(p, _BX).y;
		_BX ^= sizeof(point_t);
		pos_at(p, _BX).x = _AX;
		pos_at(p, _BX).y = _CX;
	}
	grcg_off();
}

#pragma codestring "\x90"

#pragma option -2

void sub_3F8A(void)
{
	register dot_t near *p;
	register int speed_threshold;
	int i;
	int velocity_x;
	int velocity_y;

	grcg_setcolor(GC_RMW, byte_1FFF8);
	byte_20168 = byte_1FFFA;
	byte_20169 = byte_1FFF8;
	_AX = FP_OFF(dots);
	p = reinterpret_cast<dot_t near *>(_AX);
	for(i = 0; i < DOT_COUNT; i++, p++) {
		if(p->flag != DF_ALIVE) {
			continue;
		}
		p->speed += word_1FFF2;
		p->angle += word_1FFF4;

		// ZUN bloat: vector2(velocity_x, velocity_y, p->angle, p->speed);
		asm {
			push	ss;
			lea 	ax, velocity_x;
			push	ax;
			push	ss;
			lea 	ax, velocity_y;
			push	ax;
			db	0xFF, 0x74, 0x01; // PUSH WORD PTR [SI + 1] (dot_t::angle)
			db	0x8A, 0x44, 0x0B; // MOV AL, [SI + 0Bh] (dot_t::speed)
			mov 	ah, 0;
			push	ax;
			nop;
			push	cs;
			call	near ptr vector2;
		}

		_BX = (page_back * sizeof(point_t));
		_DX = velocity_x;
		pos_at(p, _BX).x += static_cast<int>(_DX);
		_DX = velocity_y;
		pos_at(p, _BX).y += static_cast<int>(_DX);
		_DX = pos_at(p, _BX).x;
		static_cast<int>(_DX) >>= 4;
		circle_dot_left = _DX;
		_AX = pos_at(p, _BX).y;
		static_cast<int>(_AX) >>= 4;
		circle_dot_top = _AX;
		if(
			(static_cast<int>(_DX) >= 416) ||
			(static_cast<int>(_DX) <= 24) ||
			(static_cast<int>(_AX) <= 8) ||
			(static_cast<int>(_AX) >= 384)
		) {
			p->flag = DF_REMOVE;
			continue;
		}
		speed_threshold = ((word_1FFF6 << 2) + word_1FFF6);
		_BL = 5;
		while(speed_threshold > word_1FFF6) {
			if(p->speed > speed_threshold) {
				break;
			}
			speed_threshold -= word_1FFF6;
			_BL--;
		}
		p->size = _BL;
		if(!reduce_effects || (page_back == (i & 1))) {
			circle_dot_put(p->size);
		}
	}
	grcg_off();
}
