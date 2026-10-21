/* E29.7 host proof: original Link initialization + entity list +
 * player hitbox + interaction, assembled WITHOUT rewriting game routines.
 * Sprite-layer and DMA functions below are deliberate HOST-ONLY boundaries.
 * This is NOT an ARM9 native spawn or a playable prototype.
 */
#include "area.h"
#include "game.h"
#include "flags.h"
#include "room.h"
#include "save.h"
#include "player.h"
#include "entity.h"
#include "common.h"
#include "dsi_entity_state_e29.h"
#include "dsi_collision_state_e29.h"
#include "dsi_player_state_e29.h"
#include <assert.h>
#include <stdio.h>
#include <string.h>

SaveFile gSave;
RoomTransition gRoomTransition;
RoomControls gRoomControls;
PlayerEntity gPlayerEntity;
PlayerState gPlayerState;
PriorityHandler gPriorityHandler;

static unsigned sprite_calls, flag_calls;
void MemClear(void *p, u32 count) { memset(p, 0, count); }
void MemFill32(u32 value, void *p, u32 count) {
    assert((count & 3u) == 0);
    for (u32 i = 0; i < count; i += 4) memcpy((u8*)p+i, &value, 4);
}
u32 CheckGlobalFlag(u32 flag) { (void)flag; ++flag_calls; return 0; }
void UpdateSpriteForCollisionLayer(Entity *e) {
    assert(e == &gPlayerEntity.base);
    ++sprite_calls;
}
void Port_LogListOp(const char *op, void *ent, unsigned kind,
                    unsigned id, unsigned listIndex, void *list) {
    (void)op; (void)ent; (void)kind; (void)id; (void)listIndex; (void)list;
}
void InitializePlayer(void);
int main(void) {
    DsiEntityState_Reset();
    DsiCollisionState_Reset();
    DsiPlayerState_Reset();
    assert(DsiEntityState_EmptyValid() && DsiCollisionState_EmptyValid());
    assert(DsiPlayerState_Ready());
    gSave.stats.health = 24;
    gSave.stats.maxHealth = 24;
    gRoomTransition.player_status.spawn_type = PL_SPAWN_DEFAULT;
    gRoomTransition.player_status.start_pos_x = 92;
    gRoomTransition.player_status.start_pos_y = 115;
    gRoomTransition.player_status.start_anim = IdleSouth;
    gRoomTransition.player_status.layer = 1;
    InitializePlayer();
    Entity *link = &gPlayerEntity.base;
    LinkedList *list = &gEntityLists[1];
    assert(sprite_calls == 1 && flag_calls == 1);
    assert(link->kind == PLAYER && link->health == 24);
    assert(link->x.HALF.HI == 92 && link->y.HALF.HI == 115);
    assert(link->collisionLayer == 1 && link->animationState == IdleSouth);
    assert((link->flags & (ENT_COLLIDE | ENT_PERSIST)) == (ENT_COLLIDE | ENT_PERSIST));
    assert((gPlayerState.flags & PL_NO_CAP) != 0);
    assert(gRoomControls.camera_target == link);
    assert(list->first == link && list->last == link);
    assert(link->next == (Entity*)(void*)list);
    assert(link->prev == (Entity*)(void*)list);
    assert(link->updatePriority == 1 && gEntCount == 1 && gManagerCount == 0);
    assert(DsiCollisionState_PlayerRegistered());
    assert(DsiPlayerState_Ready());
    assert(gPlayerState.path_memory[0] == 0xffffffffu);
    /* Cleanup before reset: never leave stale list pointers or hitboxes. */
    DsiEntityState_Reset();
    DsiCollisionState_Reset();
    DsiPlayerState_Reset();
    assert(DsiEntityState_EmptyValid() && DsiCollisionState_EmptyValid());
    assert(!DsiCollisionState_PlayerRegistered() && DsiPlayerState_Ready());
    puts("PASS E29.7: genuine original InitializePlayer -> genuine list + hitbox + interactions (host)");
    return 0;
}
