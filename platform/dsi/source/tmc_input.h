#ifndef TMC_INPUT_H
#define TMC_INPUT_H
#include <stdint.h>
/* GBA KEYINPUT register is active low. DS keys A/B/Select/Start/D-pad/L/R
 * occupy the same lower ten bits. Other NDS keys are deliberately ignored. */
uint16_t TmcInput_MapNdsToGba(uint32_t held_nds_keys);
#endif
