/* Regression of the runtime E30.7 transaction with ORIGINAL upstream
 * intrusive list, priority, duplicate find and registration functions.
 * Real Zelda NPCs, scripts and graphics are NEVER faked in a shipped NDS.
 */
#include "dsi_room_registry_probe_e30.h"
#include "dsi_entity_state_e29.h"
#include "entity.h"
#include "room.h"
#include "player.h"
#include "game.h"
#include "common.h"
#include <assert.h>
#include <stdio.h>
#include <string.h>
RoomTransition gRoomTransition;
PlayerEntity gPlayerEntity;
PriorityHandler gPriorityHandler;
UpdateContext gUpdateContext;
static bool native_ready;
bool DsiRoomPropertyNativeE30_Ready(void) { return native_ready; }
void MemCopy(const void *src, void *dst, u32 size) {
    memcpy(dst, src, size);
}
void Port_LogListOp(const char *op,void *ent,unsigned kind,unsigned id,
                    unsigned li,void *list) {
    (void)op; (void)ent; (void)kind; (void)id; (void)li; (void)list;
}
int main(void) {
    DsiEntityState_Reset();
    DsiRoomRegistryProbeE30_Reset();
    memset(&gPlayerEntity,0,sizeof gPlayerEntity);
    gPlayerEntity.base.kind=PLAYER;
    gPlayerEntity.base.id=3;
    gPlayerEntity.base.health=22;
    AppendEntityToList(&gPlayerEntity.base,1);
    assert(gEntCount==1u);
    assert(!DsiRoomRegistryProbeE30_Run());
    assert(!DsiRoomRegistryProbeE30_Passed());
    native_ready=true;
    assert(DsiRoomRegistryProbeE30_Run());
    assert(DsiRoomRegistryProbeE30_Passed());
    assert(gEntCount==1u && gManagerCount==0u);
    assert(gPlayerEntity.base.health==22);
    assert(gEntityLists[1].first == &gPlayerEntity.base &&
           gEntityLists[1].last == &gPlayerEntity.base);
    assert(gEntityLists[7].first==(Entity*)(void*)&gEntityLists[7] &&
           gEntityLists[7].last==(Entity*)(void*)&gEntityLists[7]);
    assert(!gEntities[0].base.prev && !gEntities[0].base.next &&
           gEntities[0].base.kind==0u);
    assert(!DsiRoomRegistryProbeE30_Run()); /* strict one-shot */
    /* Reset may rearm, but do not run again against old list states. */
    DsiRoomRegistryProbeE30_Reset();
    native_ready=false;
    assert(!DsiRoomRegistryProbeE30_Run());
    assert(!DsiRoomRegistryProbeE30_Passed());
    puts("PASS E30.7: original RegisterRoomEntity executed, lists rolled back, Link preserved");
    return 0;
}
