/* E29.1: ORIGINAL InitializePlayer() on host; no player emulator.
 * Device collision, sprite layer and intrusive lists are TEST BOUNDARIES.
 */
#include "area.h"
#include "flags.h"
#include "game.h"
#include "room.h"
#include "save.h"
#include "player.h"
#include "entity.h"
#include "common.h"
#include <assert.h>
#include <stdio.h>
#include <string.h>

SaveFile gSave;
RoomTransition gRoomTransition;
RoomControls gRoomControls;
PlayerEntity gPlayerEntity;
PlayerState gPlayerState;
ItemBehavior gActiveItems[MAX_ACTIVE_ITEMS];

static unsigned interactions, sprite_layers, list_appends, hitboxes, flag_checks;
static Entity *appended;

void MemClear(void *p, u32 len) { memset(p, 0, len); }
void MemFill32(u32 value, void *p, u32 len) {
    assert((len & 3) == 0);
    unsigned char *b = p;
    for (u32 i=0;i<len;i+=4) memcpy(b+i,&value,4);
}
void ResetPossibleInteraction(void) { ++interactions; }
bool32 CheckGlobalFlag(u32 flag) { ++flag_checks; return 0; }
void UpdateSpriteForCollisionLayer(Entity *p) {
    assert(p == &gPlayerEntity.base);
    ++sprite_layers;
}
void AppendEntityToList(Entity *p, u32 list) {
    assert(p == &gPlayerEntity.base && list == 1u);
    appended = p;
    ++list_appends;
}
void RegisterPlayerHitbox(void) { ++hitboxes; }
void InitializePlayer(void);

int main(void) {
    gSave.stats.health = 24;
    gRoomTransition.player_status.spawn_type = PL_SPAWN_DEFAULT;
    gRoomTransition.player_status.start_pos_x = 92;
    gRoomTransition.player_status.start_pos_y = 115;
    gRoomTransition.player_status.start_anim = IdleSouth;
    gRoomTransition.player_status.layer = 1;
    memset(&gPlayerEntity, 0xC5, sizeof gPlayerEntity);
    memset(&gPlayerState, 0xC5, sizeof gPlayerState);
    memset(gActiveItems, 0xC5, sizeof gActiveItems);
    InitializePlayer();
    assert(interactions==1 && flag_checks==1 && sprite_layers==1);
    assert(list_appends==1 && hitboxes==1 && appended==&gPlayerEntity.base);
    assert(gRoomControls.camera_target==&gPlayerEntity.base);
    assert(gPlayerEntity.base.kind==PLAYER);
    assert((gPlayerEntity.base.flags & (ENT_COLLIDE|ENT_PERSIST))==(ENT_COLLIDE|ENT_PERSIST));
    assert(gPlayerEntity.base.health==24);
    assert(gPlayerEntity.base.x.HALF.HI==92 && gPlayerEntity.base.y.HALF.HI==115);
    assert(gPlayerEntity.base.animationState==IdleSouth);
    assert(gPlayerEntity.base.collisionLayer==1);
    assert((gPlayerState.flags & PL_NO_CAP)!=0);
    for(unsigned i=0;i<sizeof gActiveItems;i++) assert(((u8*)gActiveItems)[i]==0);
    assert(gPlayerState.path_memory[0]==0xffffffffu);
    /* Original walking spawn state; no substitute movement logic. */
    gRoomTransition.player_status.spawn_type=PL_SPAWN_WALKING;
    InitializePlayer();
    assert(gPlayerEntity.base.speed==224u);
    assert(list_appends==2 && hitboxes==2);
    puts("PASS E29.1: original InitializePlayer sets real player entity and spawn state");
    return 0;
}
