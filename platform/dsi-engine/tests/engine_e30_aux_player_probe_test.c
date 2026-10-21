/* E30.10: actual original auxiliary player slot search and exhaustion.
 * Only host fixture marks occupied status to cover fallback, and restores
 * all bytes; no such mutation in the shipped DSi diagnostic.
 */
#include "dsi_aux_player_probe_e30.h"
#include "dsi_entity_state_e29.h"
#include "entity.h"
#include "room.h"
#include "player.h"
#include "game.h"
#include <assert.h>
#include <stdint.h>
#include <stdio.h>
#include <string.h>
RoomTransition gRoomTransition;
PriorityHandler gPriorityHandler;
PlayerEntity gPlayerEntity;
static bool managerReady, auditReady;
bool DsiManagerPoolProbeE30_Passed(void) { return managerReady; }
bool DsiRoomSpawnAuditE30_Ready(void) { return auditReady; }
void Port_LogListOp(const char*op,void*e,unsigned k,unsigned id,unsigned list,void*listptr) {
    (void)op;(void)e;(void)k;(void)id;(void)list;(void)listptr;
}
int main(void) {
    DsiEntityState_Reset();
    DsiAuxPlayerProbeE30_Reset();
    gPlayerEntity.base.kind=PLAYER;
    gPlayerEntity.base.id=0x19u;
    gPlayerEntity.base.health=24;
    AppendEntityToList(&gPlayerEntity.base,1);
    assert(!DsiAuxPlayerProbeE30_Run());
    managerReady=true;
    assert(!DsiAuxPlayerProbeE30_Run());
    auditReady=true;
    assert(DsiAuxPlayerProbeE30_Run());
    assert(DsiAuxPlayerProbeE30_Passed());
    assert(!DsiAuxPlayerProbeE30_Run());
    assert(gPlayerEntity.base.health==24 && gEntCount==1u);
    /* Check ORIGINAL allocator's full 7-slot behavior with no gameplay. */
    for (unsigned i=0;i<MAX_AUX_PLAYER_ENTITIES;++i) {
        gAuxPlayerEntities[i].base.prev=(Entity*)(void*)&gEntityLists[0];
    }
    assert(CreateAuxPlayerEntity()==NULL);
    gAuxPlayerEntities[3].base.prev=NULL;
    assert(CreateAuxPlayerEntity()==&gAuxPlayerEntities[3].base);
    assert(gAuxPlayerEntities[3].base.kind==0u &&
           gAuxPlayerEntities[3].base.prev==NULL);
    DsiEntityState_Reset();
    assert(DsiEntityState_EmptyValid());
    puts("PASS E30.10: real CreateAuxPlayerEntity linked, empty/exhausted pools and Link invariant");
    return 0;
}
