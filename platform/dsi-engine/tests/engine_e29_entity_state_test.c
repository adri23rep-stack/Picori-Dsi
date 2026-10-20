/* E29.4b: test actual RAM pools with ORIGINAL src/entity.c list operations.
 * The player instance below is test-only; no fake game object is shipped. */
#include "dsi_entity_state_e29.h"
#include "entity.h"
#include "room.h"
#include "game.h"
#include <assert.h>
#include <stdio.h>
#include <string.h>
RoomTransition gRoomTransition;
PriorityHandler gPriorityHandler;
void Port_LogListOp(const char *op, void *ent, unsigned kind, unsigned id,
                    unsigned index, void *list) {
    (void)op; (void)ent; (void)kind; (void)id; (void)index; (void)list;
}
void AppendEntityToList(Entity*, u32);
void SetInitializationPriority(void);

int main(void) {
    DsiEntityState_Reset();
    assert(DsiEntityState_EmptyValid());
    for (unsigned i=0; i<9; ++i) {
        assert(gEntityLists[i].first==(Entity*)(void*)&gEntityLists[i]);
        assert(gEntityLists[i].last==(Entity*)(void*)&gEntityLists[i]);
    }
    Entity *player=&gEntities[0].base;
    Entity *npc=&gEntities[1].base;
    player->kind=PLAYER; npc->kind=NPC;
    AppendEntityToList(player,1);
    AppendEntityToList(npc,1);
    assert(gEntityLists[1].first==player && gEntityLists[1].last==npc);
    assert(player->next==npc && npc->prev==player);
    assert(npc->next==(Entity*)(void*)&gEntityLists[1]);
    assert(player->updatePriority==1 && npc->updatePriority==1);
    assert(gEntCount==2u && gManagerCount==0u);
    assert(!DsiEntityState_EmptyValid());
    SetInitializationPriority();
    assert(gPriorityHandler.event_priority==PRIO_HIGHEST);
    DsiEntityState_Reset();
    assert(DsiEntityState_EmptyValid());
    assert(gEntities[0].base.kind==0u && gEntCount==0u);
    puts("PASS E29.4b: original entity globals and source-linked list functions");
    return 0;
}
