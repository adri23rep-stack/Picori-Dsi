/* E22: original EU area tileSet/tiles lookup. No reconstructed tileset:
 * this follows gAreaTileSets and gAreaTiles from the retail ROM and executes
 * their genuine MapDataDefinition records through the bounded ROM bridge. */
#include "dsi_room_tiles.h"
#include "dsi_rom_refs.h"
#include <string.h>
#define EU_AREA_TILESETS 0x08101BC8u /* kRomOffsets_EU.areaTileSets */
#define EU_AREA_TILES    0x081027F8u /* kRomOffsets_EU.areaTiles */
#define ROM_START 0x08000000u
#define ROM_END   0x09000000u
static bool rom_ptr(uint32_t a) {
    return a >= ROM_START && a < ROM_END && !(a & 3u);
}
bool DsiRoomTiles_LoadEU(const DsiRoomMetadata *room, DsiRoomTileStats *stats) {
    uint32_t subtable, tileset, tiles;
    DsiRoomMetadata info;
    DsiRoomTileStats tmp = {0};
    if (!room || !stats || room->area >= 0x90u ||
        room->tileset_id >= 128u || /* bounded table index */
        !DsiRomRefs_RomArrayEntry(EU_AREA_TILESETS, room->area, &subtable) ||
        !rom_ptr(subtable) ||
        !DsiRomRefs_RomArrayEntry(subtable, room->tileset_id, &tileset) ||
        !rom_ptr(tileset) ||
        !DsiRomRefs_RomArrayEntry(EU_AREA_TILES, room->area, &tiles) ||
        !rom_ptr(tiles)) return false;
    info = *room;
    info.room_map_address = tileset;
    if (!DsiRoomMaps_LoadEU(&info, &tmp.tileset)) return false;
    info.room_map_address = tiles;
    if (!DsiRoomMaps_LoadEU(&info, &tmp.tiles)) return false;
    *stats = tmp;
    return true;
}
