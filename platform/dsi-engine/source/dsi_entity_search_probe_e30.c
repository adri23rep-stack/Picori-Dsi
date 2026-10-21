/* E30.11: ORIGINAL src/entity.c::FindEntityByID, FindEntity,
 * FindNextDuplicateID, EntityHasDuplicateID — never fabricated search
 * logic. Probe only observes existing Link; no fabricated NPCs in NDS.
 */
#include "dsi_entity_search_probe_e30.h"
#include "dsi_aux_player_probe_e30.h"
#include "entity.h"
#include "player.h"
#include <stdint.h>

static bool sAttempted;
static bool sPassed;

/* Upstream declarations vary across ports, keep exact source signatures. */
extern bool32 EntityHasDuplicateID(Entity *);
extern Entity *FindEntityByID(u32 kind, u32 id, u32 listIndex);
extern Entity *FindEntity(u32 kind, u32 id, u32 listIndex, u32 type, u32 type2);
extern Entity *FindNextDuplicateID(Entity *ent, int listIndex);

void DsiEntitySearchProbeE30_Reset(void) {
    sAttempted = false;
    sPassed = false;
}
bool DsiEntitySearchProbeE30_Passed(void) { return sPassed; }

bool DsiEntitySearchProbeE30_Run(void) {
    if (sAttempted || !DsiAuxPlayerProbeE30_Passed() ||
        gEntCount != 1u || gManagerCount != 0u)
        return false;
    sAttempted = true;
    LinkedList *list = &gEntityLists[1];
    LinkedList *npcList = &gEntityLists[7];
    Entity *link = &gPlayerEntity.base;
    Entity *sentinel = (Entity *)(void *)list;
    Entity *npcSentinel = (Entity *)(void *)npcList;
    if (link->kind != PLAYER || list->first != link ||
        list->last != link || link->prev != sentinel ||
        link->next != sentinel ||
        npcList->first != npcSentinel || npcList->last != npcSentinel ||
        DeepFindEntityByID(PLAYER, link->id) != link)
        return false;
    bool ok = FindEntityByID(PLAYER, link->id, 1u) == link &&
              FindEntity(PLAYER, link->id, 1u,
                         link->type, link->type2) == link &&
              FindEntityByID(PLAYER, (u8)(link->id ^ 0x80u), 1u) == NULL &&
              FindEntityByID(PLAYER, link->id, 7u) == NULL &&
              FindEntity(NPC, link->id, 1u,
                         link->type, link->type2) == NULL &&
              FindNextDuplicateID(link, 1) == NULL &&
              !EntityHasDuplicateID(link) &&
              list->first == link && list->last == link &&
              link->prev == sentinel && link->next == sentinel &&
              npcList->first == npcSentinel &&
              npcList->last == npcSentinel &&
              gEntCount == 1u && gManagerCount == 0u;
    sPassed = ok;
    return ok;
}
