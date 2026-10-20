/* E27.2: pin ORIGINAL EU room resource descriptors in native ARM9 RAM.
 * No ROM pointers escape into C, and no Zelda gameplay is implemented here.
 * E28 will consume the verified descriptors when porting original InitRoom.
 */
#include "dsi_room_resources.h"
#include "dsi_rom_refs.h"
#include "dsi_gba_bus.h"
#include <string.h>

#define EU_AREA_TILESETS 0x08101BC8u
#define EU_AREA_TILES    0x081027F8u
#define EU_ROM_LIMIT     0x09000000u
#define MAP_MULTIPLE     0x80000000u
#define MAX_BYTES        0x8000u

static DsiRoomResources sActive;

static uint32_t le32(const uint8_t *b) {
    return (uint32_t)b[0] | ((uint32_t)b[1] << 8) |
           ((uint32_t)b[2] << 16) | ((uint32_t)b[3] << 24);
}

/* Read the 12-byte GBA MapDataDefinition format into native RAM.
 * The PC_PORT struct has a different layout; do NOT memcpy it directly.
 * The packed high bit of src means that another record follows. */
static bool pin_stream(uint32_t gba, DsiOriginalMapStream *out) {
    DsiOriginalMapStream tmp;
    memset(&tmp, 0, sizeof tmp);
    if (!out || (gba & 3u) || !DsiRomRefs_IsRomSpan(gba, 12u)) return false;
    tmp.gba_address = gba;
    for (unsigned i = 0; i < DSI_ROOM_STREAM_MAX; ++i) {
        uint8_t b[12];
        uint32_t src, dst, size;
        uint64_t at = (uint64_t)gba + (uint64_t)i * sizeof b;
        if (at >= EU_ROM_LIMIT ||
            !DsiRomRefs_IsRomSpan((uint32_t)at, sizeof b) ||
            !DsiGbaBus_Read((uint32_t)at, b, sizeof b)) return false;
        src = le32(b);
        dst = le32(b + 4);
        size = le32(b + 8);
        if (dst == 0u) {
            /* Exactly the original palette-group command form. */
            if (size != 0u || (src & ~MAP_MULTIPLE) >= 207u) return false;
        } else {
            uint32_t length = size & 0x7FFFFFFFu;
            if (length == 0u || length > MAX_BYTES ||
                !DsiGbaBus_IsWritable(dst, length)) return false;
        }
        tmp.command[i].src = src;
        tmp.command[i].dest = dst;
        tmp.command[i].size = size;
        tmp.count = (uint8_t)(i + 1u);
        if (!(src & MAP_MULTIPLE)) {
            *out = tmp; /* publish only a terminated complete stream */
            return true;
        }
    }
    return false; /* unbounded / corrupt chain */
}

void DsiRoomResources_Reset(void) {
    memset(&sActive, 0, sizeof sActive);
}
const DsiRoomResources *DsiRoomResources_Current(void) {
    return sActive.valid ? &sActive : NULL;
}

bool DsiRoomResources_PrepareEU(const DsiRoomMetadata *room) {
    uint32_t subtable;
    DsiRoomResources tmp;
    if (!room || room->area >= 0x90u || room->room >= 64u ||
        room->tileSet_id >= 128u || !room->width || !room->height ||
        (room->width & 15u) || (room->height & 15u) ||
        room->width > 1024u || room->height > 1024u ||
        !DsiRomRefs_IsRomSpan(room->room_map_address, 12u) ||
        !DsiRomRefs_IsRomSpan(room->properties_address, 8u)) return false;

    memset(&tmp, 0, sizeof tmp);
    tmp.area = room->area;
    tmp.room = room->room;
    tmp.map_x = room->map_x;
    tmp.map_y = room->map_y;
    tmp.width = room->width;
    tmp.height = room->height;
    tmp.tileSet_id = room->tileset_id;
    tmp.room_map_address = room->room_map_address;
    tmp.properties_address = room->properties_address;
    for (unsigned i = 0; i < 2; ++i) {
        if (!DsiRomRefs_IsRomSpan(room->entity_list_addresses[i], 16u) ||
            room->entity_counts[i] > 128u) return false;
        tmp.entity_list_addresses[i] = room->entity_list_addresses[i];
        tmp.entity_counts[i] = room->entity_counts[i];
    }

    if (!DsiRomRefs_RomArrayEntry(EU_AREA_TILESETS, room->area, &subtable) ||
        !DsiRomRefs_RomArrayEntry(subtable, room->tileSet_id, &tmp.tileset_address) ||
        !DsiRomRefs_RomArrayEntry(EU_AREA_TILES, room->area, &tmp.area_tiles_address) ||
        !pin_stream(room->room_map_address, &tmp.map) ||
        !pin_stream(tmp.tileset_address, &tmp.tileset) ||
        !pin_stream(tmp.area_tiles_address, &tmp.tiles))
        return false;
    tmp.valid = true;
    sActive = tmp; /* atomic commit; previous snapshot retained on failure */
    return true;
}
