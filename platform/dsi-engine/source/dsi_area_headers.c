/* E28.1 — copy authentic room headers from the EU cartridge ROM into RAM.
 * Original InitAllRoomResInfo() walks these ten-byte records. The GBA
 * addresses cannot be used as host C pointers on ARM9.
 */
#include "dsi_area_headers.h"
#include "dsi_rom_refs.h"
#include "dsi_gba_bus.h"
#include <string.h>
#define EU_ROOM_HEADER_AREA_TABLE 0x0811D95Cu
static DsiAreaHeaders sActive;
static uint16_t le16(const uint8_t *p) {
    return (uint16_t)(p[0] | ((uint16_t)p[1] << 8));
}
void DsiAreaHeaders_Reset(void) { memset(&sActive, 0, sizeof sActive); }
const DsiAreaHeaders *DsiAreaHeaders_Current(void) {
    return sActive.ready ? &sActive : NULL;
}
const RoomHeader *DsiAreaHeaders_Get(uint8_t area, uint8_t room) {
    const DsiAreaHeaders *s = DsiAreaHeaders_Current();
    if (!s || area != s->area || room >= s->count) return NULL;
    return &s->rooms[room];
}
bool DsiAreaHeaders_PrepareEU(uint8_t area) {
    DsiAreaHeaders staged;
    uint32_t table;
    bool terminated = false;
    if (area >= 0x90u ||
        !DsiRomRefs_RomArrayEntry(EU_ROOM_HEADER_AREA_TABLE, area, &table))
        return false;
    memset(&staged, 0, sizeof staged);
    staged.area = area;
    for (unsigned room = 0; room < DSI_E28_MAX_ROOMS; ++room) {
        const uint64_t address = (uint64_t)table + 10u * room;
        uint8_t bytes[10];
        if (address > UINT32_MAX ||
            !DsiRomRefs_IsRomSpan((uint32_t)address, sizeof bytes) ||
            !DsiGbaBus_Read((uint32_t)address, bytes, sizeof bytes))
            return false; /* never publish a partial area */
        if (le16(bytes) == 0xFFFFu) {
            terminated = true;
            break; /* same sentinel as original InitAllRoomResInfo() */
        }
        RoomHeader *h = &staged.rooms[room];
        h->map_x = le16(bytes);
        h->map_y = le16(bytes + 2);
        h->pixel_width = le16(bytes + 4);
        h->pixel_height = le16(bytes + 6);
        h->tileSet_id = le16(bytes + 8);
        ++staged.count;
    }
    if (staged.count == 0) return false;
    /* Preserve the exact 0xFFFF sentinel contract of the original
     * InitAllRoomResInfo loop. Without this, the zero-filled native
     * tail would be mistaken for more valid rooms. */
    if (terminated && staged.count < DSI_E28_MAX_ROOMS)
        staged.rooms[staged.count].map_x = 0xFFFFu;
    /* Both a sentinel and a full 64-room table are legal for the original.
     * Retain exact ROM bytes rather than inventing rooms or dimensions. */
    (void)terminated;
    staged.ready = true;
    sActive = staged;
    return true;
}
