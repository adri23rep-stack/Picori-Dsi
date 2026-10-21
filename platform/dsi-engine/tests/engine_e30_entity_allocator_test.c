/* E30.6: invoke the ACTUAL unchanged GetEmptyEntity / GetEmptyManager /
 * GetEmptyEntityByKind / RegisterRoomEntity / AppendEntityToList.
 * Only test-only callbacks are the unreachable recycled/deletion boundary,
 * logging, and MemCopy host-memory operation. No gameplay scripts or NPC AI.
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

RoomTransition gRoomTransition;
PlayerEntity gPlayerEntity;
PriorityHandler gPriorityHandler;
UpdateContext gUpdateContext;
void DeleteEntity(Entity* entity) { (void)entity; assert(!"recycled entity outside E30.5 scope"); }
void ClearDeletedEntity(Entity* entity) { (void)entity; assert(!"deleted entity outside E30.5 scope"); }
void MemCopy(const void* src, void* dst, u32 size) { memcpy(dst, src, size); }
void Port_LogListOp(const char* op,void* ent,unsigned kind,unsigned id,
                    unsigned listIndex,void* list) {
    (void)op;(void)ent;(void)kind;(void)id;(void)listIndex;(void)list;
}
extern void* GetEmptyEntityByKind(u32 kind);
extern const u8 gEntityListLUT[];
extern Manager* GetEmptyManager(void);
extern void RegisterRoomEntity(Entity*,const EntityData*);
int main(void) {
    DsiEntityState_Reset();
    assert(DsiEntityState_EmptyValid());
    assert(gEntityListLUT[NPC]==7 && gEntityListLUT[MANAGER]==6);
    /* Link lives in its own global; original list registration preserves it. */
    Entity *link=&gPlayerEntity.base;
    link->kind=PLAYER; link->health=20; link->id=1;
    AppendEntityToList(link,1);
    assert(gEntityLists[1].first==link && gEntityLists[1].last==link);
    assert(gEntCount==1u);
    assert(DeepFindEntityByID(PLAYER, link->id) == link);
    assert(DeepFindEntityByID(PLAYER, link->id ^ 0x80u) == NULL);
    EntityData record={0};
    record.kind=NPC; record.flags=0x0f; record.id=0x2a; record.type=3;
    record.type2=0x11223344u; record.xPos=50;record.yPos=65;
    record.spritePtr=0x0800aabb;
    Entity* npc=GetEmptyEntityByKind(NPC);
    assert(npc==&gEntities[0].base);
    assert(npc!=link && !npc->prev && !npc->next);
    npc->kind=NPC;npc->id=record.id;npc->type=record.type;
    RegisterRoomEntity(npc,&record);
    assert(gEntCount==2 && gManagerCount==0);
    assert(gEntityLists[7].first==npc && gEntityLists[7].last==npc);
    assert(npc->prev==(Entity*)(void*)&gEntityLists[7]);
    assert(npc->next==(Entity*)(void*)&gEntityLists[7]);
    assert(npc->updatePriority==1);
    assert(DeepFindEntityByID(NPC, record.id) == npc);
    assert(DeepFindEntityByID(NPC, record.id ^ 0x80u) == NULL);
    assert(link->kind==PLAYER && link->health==20 &&
           gEntityLists[1].first==link && gEntityLists[1].last==link &&
           link->prev==(Entity*)(void*)&gEntityLists[1] &&
           link->next==(Entity*)(void*)&gEntityLists[1]);
    /* Original allocator naturally skips the now-registered first slot. */
    Entity* second=GetEmptyEntityByKind(OBJECT);
    assert(second==&gEntities[1].base);
    assert(second!=npc && second!=link);
    assert(GetEmptyManager()==GetEmptyEntityByKind(MANAGER));
    Manager* mgr=GetEmptyEntityByKind(MANAGER);
    assert(mgr);
    assert(mgr->prev==NULL);
    EntityData md={0};
    md.kind=MANAGER;md.id=5;md.flags=0x08;md.type=2;
    md.type2=0x01020304;md.spritePtr=0x08005000;
    mgr->kind=MANAGER;mgr->id=md.id;mgr->type=md.type;
    RegisterRoomEntity((Entity*)mgr,&md);
    assert(gManagerCount==1 && gEntCount==2);
    assert(gEntityLists[8].first==(Entity*)mgr &&
           gEntityLists[8].last==(Entity*)mgr);
    assert(GetEmptyManager()!=mgr);
    assert(DeepFindEntityByID(MANAGER, md.id) == (Entity*)mgr);
    assert(DeepFindEntityByID(MANAGER, md.id ^ 0x80u) == NULL);
    assert(link->kind==PLAYER && link->health==20 && npc->kind==NPC);
    /* The original search never allocates or mutates active lists. */
    assert(gEntCount == 2 && gManagerCount == 1);
    assert(gEntityLists[1].first == link && gEntityLists[1].last == link);
    puts("PASS E30.6: original allocator, manager pool, registry and ID search preserve Link");
    return 0;
}
