/* E30.8: inspect original EU EntityData as data ONLY. Not a spawn
 * routine. The flags below match branch conditions in UNMODIFIED
 * src/room.c LoadRoomEntity/sub_0804AF0C, with no AI or scripts added.
 */
#include "dsi_room_spawn_audit_e30.h"
#include "dsi_room_property_native_e30.h"
#include "dsi_room_property_data_e30.h"
#include "entity.h"
#include "room.h"
#include <string.h>

static DsiRoomSpawnAuditE30 sStats;
static bool sReady;

void DsiRoomSpawnAuditE30_Reset(void) {
    memset(&sStats, 0, sizeof sStats);
    sReady = false;
}
const DsiRoomSpawnAuditE30 *DsiRoomSpawnAuditE30_Get(void) {
    return sReady ? &sStats : NULL;
}
bool DsiRoomSpawnAuditE30_Ready(void) {
    return sReady && DsiRoomPropertyNativeE30_Ready() &&
           gEntCount == 1u && gManagerCount == 0u;
}
bool DsiRoomSpawnAuditE30_Run(void) {
    if (sReady || !DsiRoomPropertyNativeE30_Ready() ||
        gEntCount != 1u || gManagerCount != 0u)
        return false;
    DsiRoomSpawnAuditE30 local = {0};
    for (unsigned bank=0; bank<DSI_ENTITY_LIST_BANKS; ++bank) {
        const EntityData *records = DsiRoomPropertyDataE30_EntityBank(bank);
        const unsigned count = DsiRoomPropertyDataE30_Count(bank);
        if (!records || count >= DSI_ENTITY_RECORD_MAX ||
            records[count].kind != 0xffu)
            return false;
        for (unsigned i=0; i<count; ++i) {
            const EntityData *d = &records[i];
            const unsigned kind = d->kind & 0x0fu;
            const unsigned mode = d->flags & 0xf0u;
            if (d->kind == 0xffu) return false;
            ++local.records;
            if (kind == MANAGER) ++local.managers;
            if (kind > MANAGER || kind == 0u || kind == 2u || kind == 5u)
                ++local.unfamiliar_kind;
            if (mode == 0x40u) ++local.scripted;
            if (mode == 0x50u) ++local.duplicate_check;
            /* Mirrors the original unresolved ResolveCollisionLayer
             * branch for non-manager entries which aren't layer-pinned. */
            if (kind != MANAGER && mode != 0x10u &&
                !(d->kind & 0x30u) &&
                !(gRoomControls.scroll_flags & 2u))
                ++local.collision;
        }
    }
    sStats = local;
    sReady = true;
    return DsiRoomSpawnAuditE30_Ready();
}
