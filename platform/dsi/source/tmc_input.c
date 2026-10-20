#include "tmc_input.h"
uint16_t TmcInput_MapNdsToGba(uint32_t held_nds_keys) {
    return (uint16_t)(~held_nds_keys) & 0x03FFu;
}
