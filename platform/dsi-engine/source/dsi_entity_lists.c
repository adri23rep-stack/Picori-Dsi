/* E28.3: native RAM snapshots of the ORIGINAL 16-byte entity records.
 * These bytes come from the EU ROM with bounds/terminator validation.
 * No synthetic Zelda entities, AI, physics, or game-loop replacements.
 */
#include "dsi_entity_lists.h"
#include "dsi_rom_refs.h"
#include "dsi_gba_bus.h"
#include <string.h>
static DsiEntityLists sActive;
static DsiEntityLists sStaged;

void DsiEntityLists_Reset(void) {
    memset(&sActive, 0, sizeof sActive);
    memset(&sStaged, 0, sizeof sStaged);
}
const DsiEntityLists *DsiEntityLists_Current(void) {
    return sActive.valid ? &sActive : NULL;
}
bool DsiEntityLists_PrepareEU(const DsiRoomMetadata *room) {
    if (!room || room->area >= 0x90u || room->room >= 64u) return false;
    memset(&sStaged, 0, sizeof sStaged);
    sStaged.area = room->area;
    sStaged.room = room->room;
    for (unsigned bank = 0; bank < DSI_ENTITY_LIST_BANKS; ++bank) {
        const uint16_t count = room->entity_counts[bank];
        const uint32_t address = room->entity_list_addresses[bank];
        if (count >= DSI_ENTITY_RECORD_MAX ||
            !DsiRomRefs_IsRomSpan(address, ((size_t)count + 1u) * DSI_ENTITY_RECORD_BYTES))
            return false;
        sStaged.count[bank] = count;
        sStaged.gba_address[bank] = address;
        if (!DsiGbaBus_Read(address, sStaged.raw[bank],
                            ((size_t)count + 1u) * DSI_ENTITY_RECORD_BYTES))
            return false;
        /* The old room scanner counted the first 0xFF kind; check every
         * preceding record and the exact terminator again before commit. */
        for (unsigned i = 0; i < count; ++i) {
            if (sStaged.raw[bank][i][0] == 0xFFu) return false;
        }
        if (sStaged.raw[bank][count][0] != 0xFFu) return false;
    }
    sStaged.valid = true;
    sActive = sStaged; /* Atomic publication: no partially loaded entity lists. */
    return true;
}
