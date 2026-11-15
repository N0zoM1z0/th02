/* ReC98
 * -----
 * TH02's .MPN tile blitter, in _TEXT between master.lib's super_zoom() and
 * pfint21.
 */

#pragma option -zC_TEXT -zPTEXT

#include <stddef.h>
#include "platform.h"
#include "x86real.h"
#include "pc98.h"
#include "planar.h"
#include "th02/formats/mpn.hpp"

#define mpn_plane_word(offset) \
	*reinterpret_cast<uint16_t near *>(_SI + offset)

void pascal mpn_put_8(screen_x_t left, vram_y_t top, int image)
{
	asm { push ds; }

	// ZUN bloat: Same as vram_offset_shift(left, top).
	_DI = top;
	_AX = (left >> BYTE_BITS);
	_DX = _DI;
	_DX <<= 6;
	_AX += _DX;
	_DX >>= 2;
	_AX += _DX;
	_DI = _AX;

	_AX = (image * sizeof(mpn_image_t));
	_DX = FP_SEG(mpn_images);
	_BX = FP_OFF(mpn_images);
	_BX += _AX;
	_DS = _DX;
	_SI = _BX;
	_CX = TILE_H;
	put_loop: {
		_ES = SEG_PLANE_B;
		_poke_(_ES, _DI, mpn_plane_word(offsetof(mpn_image_t, B)));
		_ES = SEG_PLANE_R;
		_poke_(_ES, _DI, mpn_plane_word(offsetof(mpn_image_t, R)));
		_ES = SEG_PLANE_G;
		_poke_(_ES, _DI, mpn_plane_word(offsetof(mpn_image_t, G)));
		_ES = SEG_PLANE_E;
		_poke_(_ES, _DI, mpn_plane_word(offsetof(mpn_image_t, E)));
		_DI += ROW_SIZE;
		_SI += (TILE_W / BYTE_DOTS);
		asm { loop put_loop; }
	}
	asm { pop ds; }
}
