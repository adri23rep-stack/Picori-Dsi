/* E53: real retail-EU sprite pointers read from the user's ROM on SD.
 * 16-byte gSpritePtrs records: port/port_rom.c (kRomOffsets_EU.spritePtrs).
 * ROM pages have no persistent native pointers. Never return cached page ptrs. */
#include "dsi_player_anim_rom_e53.h"
#include "dsi_gba_bus.h"
#include "dsi_rom_refs.h"
#include "port_config.h"
#include <string.h>

#define EU_SPRITE_PTR_TABLE 0x08002A5Cu
#define SPRITE_RECORD_BYTES 16u
#define MAX_ANIM_INDEX 255u

static uint32_t read32(const uint8_t *p) {
    return (uint32_t)p[0] | (uint32_t)p[1] << 8 |
           (uint32_t)p[2] << 16 | (uint32_t)p[3] << 24;
}
bool DsiPlayerAnimE53_Preflight(uint32_t sprite_index,
                                uint32_t animation_index,
                                DsiPlayerAnimE53 *result) {
    uint8_t record[SPRITE_RECORD_BYTES], frame[4];
    uint32_t anim_base, frames_base, tiles_base, anim_ptr;
    if (!result || sprite_index >= PORT_EU_SPRITE_PTR_COUNT ||
        animation_index > MAX_ANIM_INDEX) return false;
    memset(result, 0, sizeof(*result));
    uint32_t entry = EU_SPRITE_PTR_TABLE + sprite_index * SPRITE_RECORD_BYTES;
    if (!DsiRomRefs_IsRomSpan(entry, sizeof(record)) ||
        !DsiGbaBus_Read(entry, record, sizeof(record))) return false;
    anim_base = read32(record);
    frames_base = read32(record + 4);
    tiles_base = read32(record + 8);
    if (!DsiRomRefs_IsRomSpan(anim_base, (animation_index + 1u) * 4u) ||
        !DsiRomRefs_IsRomSpan(frames_base, 4u) ||
        !DsiRomRefs_IsRomSpan(tiles_base, 32u)) return false;
    if (!DsiRomRefs_ReadU32(anim_base + animation_index * 4u, &anim_ptr) ||
        !DsiRomRefs_IsRomSpan(anim_ptr, sizeof(frame)) ||
        !DsiGbaBus_Read(anim_ptr, frame, sizeof(frame))) return false;
    result->animations_gba = anim_base;
    result->frames_gba = frames_base;
    result->tiles_gba = tiles_base;
    result->animation_gba = anim_ptr;
    result->frame_index = frame[0];
    result->duration = frame[1];
    result->sprite_settings = frame[2];
    result->flags = frame[3];
    return true;
}
