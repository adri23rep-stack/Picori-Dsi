/* E30 guard tests deliberately use synthetic globals ONLY in host tests.
 * The native code must reject them until original E29 actually runs.
 */
#include "dsi_e30_gate.h"
#include "dsi_game_native.h"
#include "main.h"
#include "game.h"
#include "area.h"
#include "entity.h"
#include "room.h"
#include "player.h"
#include <assert.h>
#include <stdio.h>
#include <string.h>
Main gMain;
Area gArea;
RoomControls gRoomControls;
PlayerEntity gPlayerEntity;
LinkedList gEntityLists[9];
u8 gEntCount;
static DsiGameNativeStats stats;
static bool collisionOK;
static bool interactionOK;
const DsiGameNativeStats *DsiGameNative_Status(void) { return &stats; }
bool DsiCollisionState_PlayerRegistered(void) { return collisionOK; }
bool DsiPlayerState_Ready(void) { return interactionOK; }
int main(void) {
    unsigned failures;
    DsiE30Gate_Reset();
    failures = DsiE30Gate_Blockers();
    assert((failures & DSI_E30_BLOCK_PLAYER) != 0u);
    assert((failures & DSI_E30_BLOCK_ENTITIES_INIT) != 0u);
    assert((failures & DSI_E30_BLOCK_SPRITE_LAYER) != 0u);
    assert(!DsiE30Gate_Ready());

    stats.room_prefix_ticks=1;
    stats.original_room_calls=1;
    gRoomControls.area=0x22;
    gRoomControls.room=0x15;
    gMain.task=TASK_GAME;
    gMain.state=GAMETASK_MAIN;
    gMain.substate=GAMEMAIN_CHANGEROOM;
    Entity *pl=&gPlayerEntity.base;
    LinkedList *list=&gEntityLists[1];
    pl->kind=PLAYER;
    pl->health=24;
    pl->flags=ENT_COLLIDE | ENT_PERSIST;
    pl->prev=(Entity*)(void*)list;
    pl->next=(Entity*)(void*)list;
    list->first=pl;
    list->last=pl;
    gRoomControls.camera_target=pl;
    gEntCount=1;
    collisionOK=true;
    interactionOK=true;
    /* This is intentionally a host test pointer, not a ROM script. */
    gArea.pCurrentRoomInfo=&gArea.currentRoomInfo;
    gArea.currentRoomInfo.properties=(void*)1;
    DsiE30Gate_OriginalEntitiesCompleted();
    failures=DsiE30Gate_Blockers();
    /* Even with idealized state, missing sprite-layer service blocks E30. */
    assert(failures == DSI_E30_BLOCK_SPRITE_LAYER);
    assert(!DsiE30Gate_Ready());
    gEntCount=0;
    assert((DsiE30Gate_Blockers() & DSI_E30_BLOCK_LIST) != 0);
    gEntCount=1;
    DsiE30Gate_Reset();
    assert((DsiE30Gate_Blockers() & DSI_E30_BLOCK_ENTITIES_INIT) != 0);
    puts("PASS E30 gate: no original sprite layer => no original GameTask UPDATE");
    return 0;
}
