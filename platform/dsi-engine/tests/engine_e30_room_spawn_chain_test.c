/* E30.8 host: exercise original LoadRoomEntity with original allocator,
 * registry, duplicate search and list traversal. Special mode 0x10
 * intentionally avoids unported scripts/VRAM/collision branches. The
 * unported service traps below exist ONLY in the host test, never ARM9.
 */
#include "dsi_entity_state_e29.h"
#include "entity.h"
#include "manager.h"
#include "room.h"
#include "player.h"
#include "game.h"
#include "common.h"
#include <assert.h>
#include <stdio.h>
#include <string.h>

RoomControls gRoomControls;
RoomTransition gRoomTransition;
PlayerEntity gPlayerEntity;
PriorityHandler gPriorityHandler;
UpdateContext gUpdateContext;
static EntityData records[4];
static unsigned resolves, log_calls;
const void* Port_ResolveRegionData(const void *p) {
    assert(p == NULL || p == (void*)&records[0] ||
           p == (void*)&records[1] || p == (void*)&records[2] ||
           p == (void*)&records[3]);
    ++resolves;
    return p;
}
void DeleteEntity(Entity* e) { (void)e; assert(!"unported DeleteEntity"); }
void ClearDeletedEntity(Entity* e) { (void)e; assert(!"unported ClearDeletedEntity"); }
void sub_0804AF0C(Entity *e,const EntityData *d) {
    (void)e; (void)d;
    assert(!"unported room initialization path executed");
}
u32 ResolveCollisionLayer(Entity *e) {
    (void)e;
    assert(!"unported collision path executed");
    return 0u;
}
void MemCopy(const void* src,void* dest,u32 size) {
    memcpy(dest,src,size);
}
void Port_LogListOp(const char *op,void *entity,unsigned kind,
                    unsigned id,unsigned list,void *addr) {
    (void)op;(void)entity;(void)kind;(void)id;(void)list;(void)addr;
    ++log_calls;
}
int main(void) {
    DsiEntityState_Reset();
    Entity* link=&gPlayerEntity.base;
    memset(link,0,sizeof gPlayerEntity);
    link->kind=PLAYER; link->id=1; link->health=24;
    AppendEntityToList(link,1);
    assert(gEntCount==1);
    /* original 0x10 special mode skips the yet-unported branch. */
    records[0].kind=NPC; records[0].flags=0x1f;
    records[0].id=0x2a; records[0].type=3; records[0].type2=0x11223344u;
    records[1].kind=NPC; records[1].flags=0x5f; /* original duplicate filter */
    records[1].id=0x2a;
    records[2].kind=OBJECT; records[2].flags=0x1f; records[2].id=0x19;
    records[3].kind=0xff;
    LoadRoomEntityList(records); /* untouched original traversal */
    Entity* npc=&gEntities[0].base;
    Entity* object=&gEntities[1].base;
    assert(npc->kind==NPC && npc->id==0x2a && npc->type==3);
    assert(object->kind==OBJECT && object->id==0x19);
    assert(gEntCount==3u && gManagerCount==0u);
    assert(gEntities[2].base.kind==0u);
    assert(gEntityLists[7].first==npc && gEntityLists[7].last==npc);
    assert(gEntityLists[6].first==object && gEntityLists[6].last==object);
    assert(DeepFindEntityByID(NPC,0x2a)==npc);
    assert(DeepFindEntityByID(OBJECT,0x19)==object);
    assert(gEntityLists[1].first==link && gEntityLists[1].last==link);
    assert(link->health==24 && link->kind==PLAYER);
    assert(resolves==4u && log_calls==1u);
    puts("PASS E30.8: genuine LoadRoomEntity chain allocates and deduplicates safe-mode fixtures on host; game spawn remains blocked on ARM9");
    return 0;
}
