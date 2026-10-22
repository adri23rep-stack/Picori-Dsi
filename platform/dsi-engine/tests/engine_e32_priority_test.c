/* E32: test original priority rules, not a replacement entity loop. */
#include "entity.h"
#include "player.h"
#include "message.h"
#include <assert.h>
#include <stdio.h>
PriorityHandler gPriorityHandler;
PlayerEntity gPlayerEntity;
Message gMessage;
extern void DsiPriorityE32_Tick(void);
int main(void) {
    Entity e = {0};
    e.kind=OBJECT; e.action=1; e.updatePriority=PRIO_PLAYER;
    assert(!EntityDisabled(&e) && !AnyPrioritySet());
    gPriorityHandler.event_priority=PRIO_PLAYER_EVENT;
    assert(AnyPrioritySet() && EntityDisabled(&e));
    e.action=0; assert(!EntityDisabled(&e));
    e.flags=ENT_DELETED; assert(EntityDisabled(&e));
    e.flags=0; e.action=1; gPriorityHandler.event_priority=PRIO_MIN;
    gMessage.state=MESSAGE_ACTIVE; assert(EntityDisabled(&e));
    e.updatePriority=PRIO_MESSAGE; assert(!EntityDisabled(&e));
    gMessage.state=0; e.updatePriority=PRIO_PLAYER;
    RequestPriority(&e);
    assert(e.updatePriorityPrev==PRIO_PLAYER && e.updatePriority==PRIO_MESSAGE);
    assert(gPriorityHandler.queued_priority==PRIO_MESSAGE && gPriorityHandler.requester==&e);
    DsiPriorityE32_Tick();
    assert(gPriorityHandler.ent_priority==PRIO_MESSAGE && !gPriorityHandler.queued_priority);
    assert(SetMinPriority(PRIO_PLAYER)==0);
    RevokePriority(&e); assert(e.updatePriority==PRIO_PLAYER);
    DsiPriorityE32_Tick();
    assert(gPriorityHandler.ent_priority==PRIO_MIN && !AnyPrioritySet());
    RequestPriorityDuration(NULL,2);
    assert(gPriorityHandler.requester==NULL && gPriorityHandler.priority_timer==2);
    DsiPriorityE32_Tick();
    assert(gPriorityHandler.ent_priority==PRIO_PLAYER && gPriorityHandler.priority_timer==1);
    DsiPriorityE32_Tick();
    assert(gPriorityHandler.ent_priority==PRIO_MIN && gPriorityHandler.priority_timer==0);
    SetPriorityTimer(5); SetPriorityTimer(3);
    assert(gPriorityHandler.priority_timer==5);
    SetPlayerEventPriority();
    assert(gPriorityHandler.event_priority==PRIO_PLAYER_EVENT);
    assert(gPlayerEntity.base.updatePriority==PRIO_PLAYER_EVENT);
    ResetPlayerEventPriority();
    assert(gPriorityHandler.event_priority==PRIO_MIN);
    assert(gPlayerEntity.base.updatePriority==PRIO_PLAYER);
    SetRoomReloadPriority();
    assert(gPriorityHandler.event_priority==PRIO_PLAYER_EVENT);
    ClearEventPriority();
    assert(gPriorityHandler.event_priority==PRIO_MIN);
    puts("E32 original EntityDisabled/priority queues/timer/events PASS");
}
