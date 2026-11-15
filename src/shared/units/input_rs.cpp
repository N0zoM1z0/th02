#include "th02/hardware/input_rs.cpp"

input_t key_det; /* ZUN symbol [MAGNet2010] */

// The sound driver flags follow in this TU. (In C, TCC would emit them as
// communal variables, in a different order.)
#include "th02/snd/snd.h"

bool snd_fm_possible;
bool snd_midi_active;
char snd_interrupt_if_midi; // has no effect
bool snd_midi_possible;
