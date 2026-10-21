/* E30.10: exercise the ORIGINAL src/entity.c::CreateAuxPlayerEntity
 * without creating a new game entity. The upstream function scans a pool
 * and clears an UNUSED slot under PC_PORT. Verify the pool is zeroed
 * first so it has no state to destroy. Nothing is linked to gameplay.
 */
#include "dsi_aux_player_probe_e30.h"
#include "dsi_manager_pool_probe_e30.h"
#include "dsi_room_spawn_audit_e30.h"
#include "entity.h"
#include "player.h"

static bool sAttempted;
static bool sPassed;

void DsiAuxPlayerProbeE30_Reset(void) {
    sAttempted = false;
    sPassed = false;
}
bool DsiAuxPlayerProbeE30_Passed(void) { return sPassed; }

bool DsiAuxPlayerProbeE30_Run(void) {
    if (sAttempted || !DsiManagerPoolProbeE30_Passed() ||
        !DsiRoomSpawnAuditE30_Ready() ||
        gEntCount != 1u || gManagerCount != 0u)
        return false;
    sAttempted = true;
    Entity *link=&gPlayerEntity.base;
    LinkedList *list=&gEntityLists[1];
    if (link->kind != PLAYER ||
        list->first != link || list->last != link ||
        link->prev != (Entity*)(void*)list ||
        link->next != (Entity*)(void*)list)
        return false;
    for (unsigned i=0;i<MAX_AUX_PLAYER_ENTITIES;++i) {
        const unsigned char *p=(const unsigned char*)(const void*)&gAuxPlayerEntities[i];
        for (unsigned b=0;b<sizeof(GenericEntity);++b)
            if (p[b]!=0u) return false;
    }
    const Entity *first=CreateAuxPlayerEntity(); /* exact source */
    const Entity *second=CreateAuxPlayerEntity(); /* no reservation */
    bool ok = first==&gAuxPlayerEntities[0].base &&
              second==first && first!=link &&
              first->kind==0u && first->prev==NULL && first->next==NULL &&
              gEntCount==1u && gManagerCount==0u &&
              list->first==link && list->last==link &&
              link->prev==(Entity*)(void*)list &&
              link->next==(Entity*)(void*)list &&
              DeepFindEntityByID(PLAYER,link->id)==link &&
              DsiRoomSpawnAuditE30_Ready();
    for (unsigned i=0;i<MAX_AUX_PLAYER_ENTITIES;++i) {
        const unsigned char *p=(const unsigned char*)(const void*)&gAuxPlayerEntities[i];
        for (unsigned b=0;b<sizeof(GenericEntity);++b)
            if (p[b]!=0u) ok=false;
    }
    sPassed=ok;
    return ok;
}
