/* ReC98
 * -----
 * The first random number ring of TH02's MAIN.EXE, a TU of its own between the
 * stage loop and cfg_load()
 */

#pragma option -zCmain_01_TEXT -zPmain_01 -G

#include "platform.h"
#include "libs/master.lib/master.hpp"
#include "th02/math/randring.hpp"

// Uninitialized data
uint8_t randring[RANDRING_SIZE];
uint8_t randring_p;

void near randring_fill(void)
{
	int i = 0;
	do {
		randring[i] = irand();
		i++;
	} while(i < RANDRING_SIZE);
}

#define RANDRING_INSTANCE 1
#include "th02/math/randring.cpp"
