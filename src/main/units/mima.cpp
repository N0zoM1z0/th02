/* ReC98
 * -----
 * Stage 5 boss code (Mima) of TH02 MAIN.EXE, followed by skill_calculate()
 * and Mima's intro and defeat code. One TU in the original: its relocations
 * share a LEDATA chunk across the boundary. The second part is compiled
 * without -G (the KAJA fade call cleans its argument with POP CX).
 */

#pragma option -zCmain_03_TEXT -zPmain_03 -G -a2

#include "th02/main/boss/mima.cpp"

#pragma option -G-

#include "th02/main/boss/b5_.cpp"
