/* Host regression for ORIGINAL four src/entity.c query functions.
 * The host creates list fixtures via the actual AppendEntityToList, but
 * the shipped DSi code never inserts these fixtures into Zelda gameplay.
 */
#include "dsi_entity_search_probe_e30.h"
#include "dsi_entity_state_e29.h"
#include "entity.h"
#include "player.h"
#include "room.h"
#include "game.h"
#include <assert.h>
#include <stdio.h>
#include <string.h>
RoomTransition gRoomTransition;
PriorityHandler gPriorityHandler;
PlayerEntity gPlayerEntity;
static bool auxReady;
bool DsiAuxPlayerProbeE30_Passed(void) { return auxReady; }
void Port_LogListOp(const char*op,void*e,unsigned k,unsigned id,unsigned index,void*p) {
    (void)op;(void)e;(void)k;(void)id;(void)index;(void)p;
}
extern bool32 EntityHasDuplicateID(Entity *);
extern Entity *FindEntityByID(u32,u32,u32);
extern Entity *FindEntity(u32,u32,u32,u32,u32);
extern Entity *FindNextDuplicateID(Entity *,int);
int main(void) {
    DsiEntityState_Reset();
    memset(&gPlayerEntity,0,sizeof gPlayerEntity);
    gPlayerEntity.base.kind=PLAYER;
    gPlayerEntity.base.id=0x13u;
    gPlayerEntity.base.health=30u;
    AppendEntityToList(&gPlayerEntity.base,1u);
    DsiEntitySearchProbeE30_Reset();
    assert(!DsiEntitySearchProbeE30_Run());
    assert(!DsiEntitySearchProbeE30_Passed());
    auxReady=true;
    assert(DsiEntitySearchProbeE30_Run());
    assert(DsiEntitySearchProbeE30_Passed());
    assert(!DsiEntitySearchProbeE30_Run());
    /* Other real-list queries with both positive and duplicate matches. */
    Entity *a=&gEntities[0].base, *b=&gEntities[1].base;
    a->kind=NPC;a->id=0x42;a->type=3;a->type2=5;
    b->kind=NPC;b->id=0x42;b->type=3;b->type2=7;
    AppendEntityToList(a,7u);
    AppendEntityToList(b,7u);
    assert(gEntCount==3u);
    assert(FindEntityByID(NPC,0x42,7)==a);
    assert(FindEntity(NPC,0x42,7,3,5)==a);
    assert(FindEntity(NPC,0x42,7,3,7)==b);
    assert(FindEntity(NPC,0x42,7,3,8)==NULL);
    assert(FindNextDuplicateID(a,7)==b);
    assert(FindNextDuplicateID(b,7)==NULL);
    assert(EntityHasDuplicateID(a) && EntityHasDuplicateID(b));
    assert(!EntityHasDuplicateID(&gPlayerEntity.base));
    assert(DeepFindEntityByID(NPC,0x42)==a);
    assert(gPlayerEntity.base.health==30u);
    DsiEntityState_Reset();
    assert(DsiEntityState_EmptyValid());
    puts("PASS E30.11: four unchanged original entity find/duplicate routines; real lists preserved");
    return 0;
}
