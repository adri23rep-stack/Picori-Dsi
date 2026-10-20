#include "dsi_room_probe.h"
#include "dsi_gba_bus.h"
#include "dsi_rom_refs.h"
#include <string.h>

#define GBA_ROM_START 0x08000000u
#define GBA_ROM_END   0x09000000u
#define ROOM_HEADER_TABLE_EU (GBA_ROM_START + 0x11D95Cu)
#define ROOM_MAP_TABLE_EU    (GBA_ROM_START + 0x1070E4u)
#define ROOM_PROPS_TABLE_EU  (GBA_ROM_START + 0x0D4828u)
#define AREA_TABLE_COUNT     0x90u
#define ROOM_HEADER_BYTES    10u
#define MAX_ENTITY_RECORDS   128u
#define ENTITY_BYTES         16u

static bool rom_range(uint32_t addr, size_t length) {
    return addr >= GBA_ROM_START && addr < GBA_ROM_END &&
           length <= (size_t)(GBA_ROM_END - addr);
}
static uint16_t le16(const unsigned char *p) {
    return (uint16_t)(p[0] | ((uint16_t)p[1] << 8));
}
static bool area_array(uint32_t base, unsigned area, uint32_t *ptr) {
    if (!DsiRomRefs_RomArrayEntry(base, area, ptr)) return false;
    return rom_range(*ptr, 4);
}
static bool room_ptr(uint32_t table, unsigned room, uint32_t *ptr) {
    if (room > (UINT32_MAX - table) / 4u || !rom_range(table + room * 4u, 4))
        return false;
    if (!DsiRomRefs_RomArrayEntry(table, room, ptr)) return false;
    return rom_range(*ptr, 1);
}
static bool entity_count(uint32_t gba_list, uint16_t *out_count) {
    if (!rom_range(gba_list, ENTITY_BYTES)) return false;
    for (unsigned i = 0; i < MAX_ENTITY_RECORDS; ++i) {
        uint32_t address = gba_list + i * ENTITY_BYTES;
        unsigned char kind;
        if (address < gba_list || !rom_range(address, ENTITY_BYTES) ||
            !DsiGbaBus_Read(address, &kind, 1)) return false;
        if (kind == 0xFFu) { *out_count = (uint16_t)i; return true; }
    }
    return false; /* malformed list: no terminator inside bounded scan */
}

bool DsiRoomProbe_ReadEU(uint8_t area, uint8_t room, DsiRoomMetadata *out) {
    uint32_t header_table, room_map_table, properties_table;
    unsigned char bytes[ROOM_HEADER_BYTES];
    uint32_t header_address;
    DsiRoomMetadata tmp;
    if (!out || area >= AREA_TABLE_COUNT) return false;
    memset(&tmp, 0, sizeof(tmp));
    if (!area_array(ROOM_HEADER_TABLE_EU, area, &header_table) ||
        !area_array(ROOM_MAP_TABLE_EU, area, &room_map_table) ||
        !area_array(ROOM_PROPS_TABLE_EU, area, &properties_table)) return false;
    if (room > (UINT32_MAX - header_table) / ROOM_HEADER_BYTES) return false;
    header_address = header_table + (uint32_t)room * ROOM_HEADER_BYTES;
    if (!rom_range(header_address, sizeof(bytes)) ||
        !DsiGbaBus_Read(header_address, bytes, sizeof(bytes))) return false;
    tmp.map_x = le16(bytes);
    tmp.map_y = le16(bytes + 2);
    tmp.width = le16(bytes + 4);
    tmp.height = le16(bytes + 6);
    tmp.tileset_id = le16(bytes + 8);
    /* Nonempty, GBA room geometries are multiples of sixteen pixels. */
    if (!tmp.width || !tmp.height ||
        (tmp.width & 15u) || (tmp.height & 15u) ||
        tmp.width > 0x400 || tmp.height > 0x400) return false;
    if (!room_ptr(room_map_table, room, &tmp.room_map_address) ||
        !room_ptr(properties_table, room, &tmp.properties_address)) return false;
    for (unsigned i = 0; i < 2; ++i) {
        if (!DsiRomRefs_RomArrayEntry(tmp.properties_address, i,
                                   &tmp.entity_list_addresses[i]) ||
            !rom_range(tmp.entity_list_addresses[i], ENTITY_BYTES) ||
            !entity_count(tmp.entity_list_addresses[i], &tmp.entity_counts[i]))
            return false;
    }
    tmp.area = area;
    tmp.room = room;
    *out = tmp; /* publish only after every read/validation passed */
    return true;
}
