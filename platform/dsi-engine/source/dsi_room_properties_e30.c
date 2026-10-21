/* E30.1: pin the ORIGINAL 8x4-byte EU room property table in ARM9 RAM.
 * This is pointer/data transport ONLY, not a substitute room initializer.
 * Important: GBA Thumb addresses (bit 0 set) MUST NOT be invoked by ARM9
 * or stored as native code pointers in RoomResInfo.properties.
 */
#include "dsi_room_properties_e30.h"
#include "dsi_rom_refs.h"
#include "dsi_gba_bus.h"
#include <string.h>

static DsiRoomPropertiesE30 sActive;

static uint32_t read_le32(const unsigned char *p) {
    return (uint32_t)p[0] | ((uint32_t)p[1] << 8) |
           ((uint32_t)p[2] << 16) | ((uint32_t)p[3] << 24);
}

void DsiRoomPropertiesE30_Reset(void) {
    memset(&sActive, 0, sizeof sActive);
}

const DsiRoomPropertiesE30 *DsiRoomPropertiesE30_Current(void) {
    return sActive.valid ? &sActive : NULL;
}

bool DsiRoomPropertiesE30_PrepareEU(const DsiRoomMetadata *room) {
    DsiRoomPropertiesE30 tmp;
    unsigned char packed[DSI_E30_PROPERTY_SLOTS * 4u];
    if (!room || room->area >= 0x90u || room->room >= 64u ||
        (room->properties_address & 3u) ||
        !DsiRomRefs_IsRomSpan(room->properties_address, sizeof packed) ||
        !DsiGbaBus_Read(room->properties_address, packed, sizeof packed))
        return false;
    memset(&tmp, 0, sizeof tmp);
    tmp.area = room->area;
    tmp.room = room->room;
    tmp.gba_table_address = room->properties_address;
    for (unsigned i=0; i<DSI_E30_PROPERTY_SLOTS; ++i) {
        const uint32_t raw = read_le32(packed + i * 4u);
        tmp.original[i] = raw;
        if (!raw) {
            if (i < 2u) return false; /* mandatory original entity lists */
            tmp.kind[i] = DSI_E30_PROPERTY_NONE;
            continue;
        }
        uint32_t decoded = raw & ~1u;
        unsigned char first_byte;
        if (!DsiRomRefs_IsRomSpan(decoded, 1u) ||
            !DsiGbaBus_Read(decoded, &first_byte, 1u)) return false;
        tmp.kind[i] = (raw & 1u) ? DSI_E30_PROPERTY_THUMB_ADDRESS :
                                     DSI_E30_PROPERTY_ROM_DATA;
        /* Entity list banks are aligned data, never executable callbacks. */
        if (i < 2u &&
            (tmp.kind[i] != DSI_E30_PROPERTY_ROM_DATA ||
             raw != room->entity_list_addresses[i]))
            return false;
    }
    /* Do not translate unknown callbacks or invoke the original game yet. */
    tmp.native_dispatch_ready = false;
    tmp.valid = true;
    sActive = tmp; /* No partially loaded or stale pointer-table publication. */
    return true;
}
