/* E21: bounded port of the packed-ROM branch of original LoadMapData() in
 * src/beanstalkSubtask.c. Resolves ORIGINAL EU map descriptors and performs
 * ROM -> GBA shadow memory transfers; no replacement map/game algorithm. */
#include "dsi_room_maps.h"
#include "dsi_gba_bus.h"
#include "dsi_rom_refs.h"
#include "dsi_title_assets.h" /* shared bounded GBA LZ77 decoder */
#include <stddef.h>
#include <string.h>

#define EU_MAP_DATA_BASE 0x08323FECu /* kRomOffsets_EU.mapDataBase */
#define MAP_CHAIN_MAX 16u
#define MAP_SIZE_MAX 0x8000u
#define MAP_MULTIPLE 0x80000000u
#define MAP_COMPRESSED 0x80000000u
#define ROM_END 0x09000000u

typedef struct {
    uint32_t source, dest, length;
    bool compressed, palette;
} Operation;

static bool source_address(uint32_t offset, uint32_t *out) {
    if (offset >= ROM_END - EU_MAP_DATA_BASE) return false;
    *out = EU_MAP_DATA_BASE + offset;
    return true;
}

bool DsiRoomMaps_LoadEU(const DsiRoomMetadata *room, DsiRoomMapStats *stats) {
    if (!room || !stats || room->area >= 0x90u ||
        room->room_map_address < 0x08000000u ||
        room->room_map_address >= ROM_END) return false;
    Operation operations[MAP_CHAIN_MAX];
    unsigned n = 0;
    uint32_t total = 0;
    bool ended = false;
    /* Validate the complete chain before transferring anything. */
    for (; n < MAP_CHAIN_MAX; ++n) {
        uint32_t at = room->room_map_address + (uint32_t)n * 12u;
        uint32_t raw_src, dest, raw_size, source;
        if (at < room->room_map_address || at >= ROM_END ||
            ROM_END - at < 12u ||
            !DsiRomRefs_ReadU32(at, &raw_src) ||
            !DsiRomRefs_ReadU32(at + 4, &dest) ||
            !DsiRomRefs_ReadU32(at + 8, &raw_size)) return false;
        uint32_t length = raw_size & ~MAP_COMPRESSED;
        bool compressed = (raw_size & MAP_COMPRESSED) != 0;
        /* ORIGINAL LoadMapData() accepts dest==0 as a palette-group
         * command; the low 16 bits of src are a palette group ID. Do not
         * pretend it is a RAM transfer (room tilesets contain one). */
        bool palette = dest == 0;
        if (palette) {
            if (raw_size || (raw_src & ~MAP_MULTIPLE) >= 207u ||
                !DsiTitleAssets_Active()) return false;
            source = raw_src & ~MAP_MULTIPLE;
            compressed = false;
            length = 0;
        } else if (length == 0 || length > MAP_SIZE_MAX ||
                   total > MAP_SIZE_MAX * MAP_CHAIN_MAX - length ||
                   !source_address(raw_src & ~MAP_MULTIPLE, &source) ||
                   !DsiGbaBus_IsWritable(dest, length)) return false;
        if (!palette && compressed) {
            uint8_t hdr[4];
            if (!DsiGbaBus_Read(source, hdr, sizeof(hdr)) || hdr[0] != 0x10 ||
                ((uint32_t)hdr[1] | ((uint32_t)hdr[2] << 8) |
                 ((uint32_t)hdr[3] << 16)) != length) return false;
        } else if (!palette) {
            /* Read the last source byte now: reject out-of-ROM ranges. */
            uint8_t last;
            if (source >= ROM_END || length > ROM_END - source ||
                !DsiGbaBus_Read(source + length - 1, &last, 1)) return false;
        }
        operations[n] = (Operation){ source, dest, length, compressed, palette };
        total += length;
        if (!(raw_src & MAP_MULTIPLE)) { ++n; ended = true; break; }
    }
    if (!ended || n == 0 || n > MAP_CHAIN_MAX) return false;
    DsiRoomMapStats result = {0};
    for (unsigned i = 0; i < n; ++i) {
        Operation *o = &operations[i];
        if (o->palette) {
            if (!DsiTitleAssets_LoadPaletteGroup(o->source)) return false;
            ++result.palette_groups;
        } else if (o->compressed) {
            uint32_t written = 0;
            if (!DsiTitleAssets_UnpackLz77(o->source, o->dest, &written) ||
                written != o->length) return false;
            ++result.lz_blocks;
        } else if (!DsiGbaBus_Copy(o->dest, o->source, o->length)) return false;
        result.uncompressed_bytes += o->length;
        ++result.descriptors;
    }
    *stats = result;
    return true;
}
