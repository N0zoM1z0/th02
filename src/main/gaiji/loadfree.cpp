#include "th02/shiftjis/fns.hpp"
#include "th02/gaiji/loadfree.h"

void near gaiji_load(void)
{
	gaiji_backup();
	gaiji_entry_bfnt(SHIFTJIS_FN);
}

void near gaiji_free(void)
{
	gaiji_restore();
}
