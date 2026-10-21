#include "dsi_player_preflight.h"
#include "dsi_entity_lists.h"
#include "dsi_room_resources.h"
#include "room.h"
#include "save.h"
#include "player.h"
#include <assert.h>
#include <stdio.h>
#include <string.h>

RoomControls gRoomControls;
RoomTransition gRoomTransition;
SaveFile gSave;
static DsiEntityLists lists;
static DsiRoomResources room;
const DsiEntityLists *DsiEntityLists_Current(void){return lists.valid ? &lists : NULL;}
const DsiRoomResources *DsiRoomResources_Current(void){return room.valid ? &room : NULL;}
int main(void) {
    lists.valid=true; room.valid=true;
    lists.area=room.area=gRoomControls.area=gRoomTransition.player_status.area_next=0x22u;
    lists.room=room.room=gRoomControls.room=gRoomTransition.player_status.room_next=0x15u;
    gRoomTransition.player_status.spawn_type=PL_SPAWN_DEFAULT;
    gRoomTransition.player_status.layer=1;
    gSave.stats.maxHealth=24;
    gSave.stats.health=24;
    lists.count[0]=room.entity_counts[0]=2;
    lists.count[1]=room.entity_counts[1]=1;
    lists.raw[0][2][0]=0xFF;
    lists.raw[1][1][0]=0xFF;
    assert(DsiPlayerPreflight_Ready());
    gRoomTransition.player_status.spawn_type=PL_SPAWN_FAST_TRAVEL + 1;
    assert(!DsiPlayerPreflight_Ready());
    gRoomTransition.player_status.spawn_type=PL_SPAWN_DEFAULT;
    lists.raw[1][1][0]=0;
    assert(!DsiPlayerPreflight_Ready());
    lists.raw[1][1][0]=0xFF;
    gSave.stats.health=0;
    assert(!DsiPlayerPreflight_Ready());
    gSave.stats.health=24;
    gRoomTransition.player_status.area_next=0x21u;
    assert(!DsiPlayerPreflight_Ready());
    gRoomTransition.player_status.area_next=0x22u;
    lists.valid=false;
    assert(!DsiPlayerPreflight_Ready());
    lists.valid=true;
    assert(DsiPlayerPreflight_Ready());
    puts("PASS E29: real save/transition/room/entity data checked before InitializePlayer");
    return 0;
}
