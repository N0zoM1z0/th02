#pragma option -zCSHARED

#include <mem.h>
#include "libs/master.lib/master.hpp"
#include "th02/formats/tile.hpp"
#include "th02/formats/mpn.hpp"
#include "th02/shiftjis/fns.hpp"

// Uninitialized data
mpn_image_t *mpn_images;
Palette8 mpn_palette;

bool mpn_show_palette_on_load = true;
uint8_t mpn_count = 0;

// Used by game_init_main().
extern const char pf_fn[] = PF_FN;

// TH04 copied this code and adjusted it to work with multiple .MPN slots.

void mpn_palette_show(void)
{
	palette_set_all(mpn_palette);
	palette_show();
}

int pascal mpn_load_palette_show(const char *fn)
{
	mpn_header_t header;

	file_ropen(fn);
	file_read(&header, sizeof(header));
	mpn_count = header.count;

	file_read(&mpn_palette, sizeof(mpn_palette));
	if(mpn_show_palette_on_load) {
		mpn_palette_show();
	}

	if(mpn_images) {
		mpn_free();
	}
	mpn_images = reinterpret_cast<mpn_image_t __seg* >(hmem_allocbyte(
		(mpn_count + 1) * sizeof(mpn_image_t)
	));
	if(!mpn_images) {
		file_close();
		return -1;
	}
	file_read(mpn_images, (mpn_count + 1) * sizeof(mpn_image_t));

	file_close();
	return 0;
}
