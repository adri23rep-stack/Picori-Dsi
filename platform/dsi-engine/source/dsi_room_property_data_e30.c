/* E30.2: copy ORIGINAL EU 16-byte EntityData records to aligned ARM9 RAM.
 * The values inside records (incl. spritePtr) retain GBA meaning; they are
 * NOT native pointers, and are never dereferenced or dispatched here.
 * No NPC/Link spawning, callback translation, script or game-loop rewrites.
 */
#include "dsi_room_property_data_e30.h"
#include "dsi_room_properties_e30.h"
#include <string.h>

_Static_assert(sizeof(EntityData) == DSI_ENTITY_RECORD_BYTES,
               "Original ROM EntityData layout must remain 16 bytes");

typedef struct {
    bool valid;
    uint8_t area, room;
    uint16_t count[DSI_ENTITY_LIST_BANKS];
    uint32_t source[DSI_ENTITY_LIST_BANKS];
    EntityData records[DSI_ENTITY_LIST_BANKS][DSI_ENTITY_RECORD_MAX + 1u];
} NativeBanks;

static NativeBanks sActive;
static NativeBanks sStaged;

void DsiRoomPropertyDataE30_Reset(void) {
    memset(&sActive, 0, sizeof sActive);
    memset(&sStaged, 0, sizeof sStaged);
}

static bool sources_match(const NativeBanks *n) {
    const DsiRoomPropertiesE30 *p = DsiRoomPropertiesE30_Current();
    const DsiEntityLists *l = DsiEntityLists_Current();
    if (!n->valid || !p || !p->valid || !l || !l->valid ||
        p->area != n->area || p->room != n->room ||
        l->area != n->area || l->room != n->room)
        return false;
    for (unsigned bank=0; bank<DSI_ENTITY_LIST_BANKS; ++bank) {
        unsigned count = n->count[bank];
        if (count >= DSI_ENTITY_RECORD_MAX ||
            l->count[bank] != count ||
            l->gba_address[bank] != n->source[bank] ||
            p->original[bank] != n->source[bank] ||
            p->kind[bank] != DSI_E30_PROPERTY_ROM_DATA ||
            l->raw[bank][count][0] != 0xFFu ||
            memcmp(n->records[bank], l->raw[bank],
                   ((size_t)count+1u)*DSI_ENTITY_RECORD_BYTES) != 0)
            return false;
    }
    return true;
}

bool DsiRoomPropertyDataE30_Bind(void) {
    const DsiRoomPropertiesE30 *p = DsiRoomPropertiesE30_Current();
    const DsiEntityLists *l = DsiEntityLists_Current();
    if (!p || !p->valid || !l || !l->valid ||
        p->area != l->area || p->room != l->room)
        return false;

    memset(&sStaged, 0, sizeof sStaged);
    sStaged.area = p->area;
    sStaged.room = p->room;
    for (unsigned bank=0; bank<DSI_ENTITY_LIST_BANKS; ++bank) {
        const unsigned count = l->count[bank];
        if (count >= DSI_ENTITY_RECORD_MAX ||
            p->kind[bank] != DSI_E30_PROPERTY_ROM_DATA ||
            p->original[bank] != l->gba_address[bank] ||
            l->raw[bank][count][0] != 0xFFu)
            return false;
        for (unsigned i=0; i<count; ++i)
            if (l->raw[bank][i][0] == 0xFFu) return false;
        sStaged.count[bank] = (uint16_t)count;
        sStaged.source[bank] = l->gba_address[bank];
        memcpy(sStaged.records[bank], l->raw[bank],
               ((size_t)count+1u)*DSI_ENTITY_RECORD_BYTES);
    }
    sStaged.valid=true;
    sActive=sStaged;
    return sources_match(&sActive);
}

bool DsiRoomPropertyDataE30_Ready(void) { return sources_match(&sActive); }

const EntityData *DsiRoomPropertyDataE30_EntityBank(unsigned bank) {
    if (bank >= DSI_ENTITY_LIST_BANKS || !DsiRoomPropertyDataE30_Ready())
        return NULL;
    return sActive.records[bank];
}
unsigned DsiRoomPropertyDataE30_Count(unsigned bank) {
    if (bank >= DSI_ENTITY_LIST_BANKS || !DsiRoomPropertyDataE30_Ready())
        return 0u;
    return sActive.count[bank];
}
