/* E30.9: exercise the unmodified original GetEmptyManager() directly
 * on the original GBA-shaped manager pool in ARM9 RAM. No test NPCs,
 * no fake manager allocator, and no persistent changes.
 */
#include "dsi_manager_pool_probe_e30.h"
#include "dsi_room_spawn_audit_e30.h"
#include "dsi_room_registry_probe_e30.h"
#include "manager.h"
#include "player.h"
#include "entity.h"
#include <stdint.h>

static bool sAttempted;
static bool sPassed;

void DsiManagerPoolProbeE30_Reset(void) {
    sAttempted = false;
    sPassed = false;
}
bool DsiManagerPoolProbeE30_Passed(void) { return sPassed; }

bool DsiManagerPoolProbeE30_Run(void) {
    if (sAttempted || !DsiRoomRegistryProbeE30_Passed() ||
        !DsiRoomSpawnAuditE30_Ready() ||
        gEntCount != 1u || gManagerCount != 0u)
        return false;
    sAttempted = true;

    LinkedList *playerList = &gEntityLists[1];
    LinkedList *npcList = &gEntityLists[7];
    Entity *link = &gPlayerEntity.base;
    if (link->kind != PLAYER || playerList->first != link ||
        playerList->last != link ||
        link->prev != (Entity *)(void *)playerList ||
        link->next != (Entity *)(void *)playerList ||
        npcList->first != (Entity *)(void *)npcList ||
        npcList->last != (Entity *)(void *)npcList ||
        gEntities[0].base.prev != NULL ||
        gEntities[0].base.next != NULL ||
        gEntities[0].base.kind != 0u)
        return false;

    Manager *first = GetEmptyManager(); /* EXACT original src/entity.c */
    Manager *again = GetEmptyManager();
    bool ok = first != NULL && again == first &&
              first->prev == NULL && first->next == NULL &&
              first->kind == 0u &&
              (void *)first != (void *)link &&
              (void *)first != (void *)&gEntities[0] &&
              gEntCount == 1u && gManagerCount == 0u &&
              DeepFindEntityByID(PLAYER, link->id) == link &&
              playerList->first == link && playerList->last == link &&
              npcList->first == (Entity *)(void *)npcList &&
              npcList->last == (Entity *)(void *)npcList &&
              gEntities[0].base.prev == NULL &&
              gEntities[0].base.kind == 0u &&
              DsiRoomSpawnAuditE30_Ready();
    sPassed = ok;
    return ok;
}
