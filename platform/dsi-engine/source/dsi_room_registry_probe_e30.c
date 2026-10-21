/* E30.7: diagnostic only. Game behavior is entirely in unmodified
 * original RegisterRoomEntity / AppendEntityToList / DeepFindEntityByID.
 * This test fixture is NOT a Zelda NPC or an alternative spawner.
 * It never runs any update, script, sprite code or gameplay collision.
 */
#include "dsi_room_registry_probe_e30.h"
#include "dsi_room_property_native_e30.h"
#include "entity.h"
#include "room.h"
#include "player.h"
#include <string.h>

extern void RegisterRoomEntity(Entity *, const EntityData *);
static bool sAttempted;
static bool sPassed;

void DsiRoomRegistryProbeE30_Reset(void) {
    sAttempted = false;
    sPassed = false;
}
bool DsiRoomRegistryProbeE30_Passed(void) { return sPassed; }

bool DsiRoomRegistryProbeE30_Run(void) {
    if (sAttempted || !DsiRoomPropertyNativeE30_Ready() ||
        gEntCount != 1u || gManagerCount != 0u) return false;
    sAttempted = true;
    LinkedList *npcList = &gEntityLists[7];
    LinkedList *playerList = &gEntityLists[1];
    Entity *npcSentinel = (Entity *)(void *)npcList;
    Entity *playerSentinel = (Entity *)(void *)playerList;
    Entity *link = &gPlayerEntity.base;
    GenericEntity *slot = &gEntities[0];
    Entity *candidate = &slot->base;
    if (npcList->first != npcSentinel || npcList->last != npcSentinel ||
        playerList->first != link || playerList->last != link ||
        link->next != playerSentinel || link->prev != playerSentinel ||
        candidate->prev != NULL || candidate->next != NULL ||
        candidate->kind != 0u ||
        DeepFindEntityByID(PLAYER, link->id) != link)
        return false;

    /* Only a throwaway *diagnostic* record, not a game NPC. The fixture
     * is never loaded from or substituted for the original room banks. */
    EntityData diagnostic = {0};
    diagnostic.kind = NPC;
    diagnostic.flags = 0x0fu; /* original LUT routes NPC -> list 7 */
    diagnostic.id = 0xe7u;
    diagnostic.type = 1u;
    diagnostic.type2 = 0x12345678u;
    diagnostic.xPos = 20u;
    diagnostic.yPos = 30u;
    candidate->kind = NPC;
    candidate->id = diagnostic.id;
    candidate->type = diagnostic.type;
    RegisterRoomEntity(candidate, &diagnostic); /* ORIGINAL src/room.c */

    bool valid = npcList->first == candidate &&
                 npcList->last == candidate &&
                 candidate->prev == npcSentinel &&
                 candidate->next == npcSentinel &&
                 candidate->updatePriority == 1u &&
                 gEntCount == 2u &&
                 gManagerCount == 0u &&
                 DeepFindEntityByID(NPC, diagnostic.id) == candidate &&
                 playerList->first == link &&
                 playerList->last == link &&
                 link->next == playerSentinel &&
                 link->prev == playerSentinel;

    /* Transaction rollback before any game tick. Never leave stack or
     * diagnostic entity linked to the real engine pool, even on failure. */
    npcList->first = npcSentinel;
    npcList->last = npcSentinel;
    memset(slot, 0, sizeof(*slot));
    gEntCount = 1u;

    valid = valid && DeepFindEntityByID(NPC, diagnostic.id) == NULL &&
            DeepFindEntityByID(PLAYER, link->id) == link &&
            playerList->first == link && playerList->last == link &&
            gManagerCount == 0u && DsiRoomPropertyNativeE30_Ready();
    sPassed = valid;
    return valid;
}
