/* Host-only proof: native C pointer lists execute src/entity.c routines.
 * No custom entity allocator or behavior is used. */
#include "entity.h"
#include "room.h"
#include "game.h"
#include <assert.h>
#include <stdio.h>
#include <string.h>

LinkedList gEntityLists[9];
RoomTransition gRoomTransition;
u8 gEntCount;
u8 gManagerCount;
void Port_LogListOp(const char *op,void *ent,unsigned kind,unsigned id,
                    unsigned listIndex,void *listAddr) {
    (void)op; (void)ent; (void)kind; (void)id; (void)listIndex; (void)listAddr;
}
void AppendEntityToList(Entity*, u32);

int main(void) {
    for (unsigned i=0; i<9; ++i) {
        gEntityLists[i].first=(Entity*)(void*)&gEntityLists[i];
        gEntityLists[i].last =(Entity*)(void*)&gEntityLists[i];
    }
    Entity player, npc;
    memset(&player,0,sizeof player);
    memset(&npc,0,sizeof npc);
    player.kind=PLAYER;
    npc.kind=NPC;
    gRoomTransition.entity_update_type=0;
    AppendEntityToList(&player,1);
    LinkedList *list=&gEntityLists[1];
    assert(list->first==&player && list->last==&player);
    assert(player.prev==(Entity*)(void*)list && player.next==(Entity*)(void*)list);
    assert(player.updatePriority==1 && player.updatePriorityPrev==1);
    assert(gEntCount==1 && player.spritePriority.b0==4);
    AppendEntityToList(&npc,1);
    assert(list->first==&player && list->last==&npc);
    assert(player.next==&npc && npc.prev==&player);
    assert(npc.next==(Entity*)(void*)list && gEntCount==2);
    gRoomTransition.entity_update_type=2;
    Entity special;
    memset(&special,0,sizeof special);
    special.kind=PLAYER;
    AppendEntityToList(&special,2);
    assert(special.updatePriority==3 && special.updatePriorityPrev==3);
    assert(gEntityLists[2].first==&special && gEntityLists[2].last==&special);
    assert(gEntCount==3 && gManagerCount==0);
    puts("PASS E29.2: original AppendEntityToList + priorities + native pointer chains");
    return 0;
}
