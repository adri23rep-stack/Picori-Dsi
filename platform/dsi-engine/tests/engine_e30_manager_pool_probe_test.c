#include "dsi_manager_pool_probe_e30.h"
#include "dsi_entity_state_e29.h"
#include "entity.h"
#include "manager.h"
#include "player.h"
#include "room.h"
#include "game.h"
#include <assert.h>
#include <stdio.h>
#include <string.h>

RoomTransition gRoomTransition;
PriorityHandler gPriorityHandler;
PlayerEntity gPlayerEntity;
static bool registry, audit;
bool DsiRoomRegistryProbeE30_Passed(void) { return registry; }
bool DsiRoomSpawnAuditE30_Ready(void) { return audit; }
void Port_LogListOp(const char *op,void *e,unsigned k,unsigned id,
                    unsigned ix,void *l) {
    (void)op;(void)e;(void)k;(void)id;(void)ix;(void)l;
}
int main(void) {
    DsiEntityState_Reset();
    DsiManagerPoolProbeE30_Reset();
    memset(&gPlayerEntity,0,sizeof(gPlayerEntity));
    gPlayerEntity.base.kind=PLAYER;
    gPlayerEntity.base.id=0x16;
    AppendEntityToList(&gPlayerEntity.base,1);
    assert(gEntCount==1 && gManagerCount==0);
    assert(!DsiManagerPoolProbeE30_Run());
    assert(!DsiManagerPoolProbeE30_Passed());
    registry=true;
    assert(!DsiManagerPoolProbeE30_Run());
    audit=true;
    assert(DsiManagerPoolProbeE30_Run());
    assert(DsiManagerPoolProbeE30_Passed());
    assert(gEntCount==1 && gManagerCount==0);
    assert(DeepFindEntityByID(PLAYER,0x16)==&gPlayerEntity.base);
    assert(!DsiManagerPoolProbeE30_Run()); /* strict one-shot */
    assert(gEntities[0].base.kind==0 && !gEntities[0].base.prev);
    puts("PASS E30.9: unmodified GetEmptyManager inspected ARM9-style manager pool; Link untouched");
    return 0;
}
