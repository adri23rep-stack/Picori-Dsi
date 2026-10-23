#ifndef DSI_PLAYER_ANIM_ROM_E53_H
#define DSI_PLAYER_ANIM_ROM_E53_H
/* Native access to ORIGINAL EU GBA sprite pointer and animation tables.
 * This validates ROM-provided frame data; it does not invent actions. */
#include <stdbool.h>
#include <stdint.h>
typedef struct {
    uint32_t animations_gba, frames_gba, tiles_gba, animation_gba;
    uint8_t frame_index, duration, sprite_settings, flags;
} DsiPlayerAnimE53;
bool DsiPlayerAnimE53_Preflight(uint32_t sprite_index,
                                uint32_t animation_index,
                                DsiPlayerAnimE53 *result);
#endif
