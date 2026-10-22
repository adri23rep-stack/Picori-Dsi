/* E38 regression: the original InitRoom RoomResInfo must be bound
 * before LoadRoomEntityList; stale/mismatched geometry never mutates state. */
#include "dsi_room_geometry_e38.h"
#include "area.h"
#include "entity.h"
#include "game.h"
#include "main.h"
#include "player.h"
#include "room.h"
#include <assert.h>
#include <stdio.h>
#include <string.h>
Area gArea;
RoomControls gRoomControls;
Main gMain;
PlayerEntity gPlayerEntity;
u32 gEntCount,gManagerCount;
int main(void) {
    DsiRoomMetadata room={0};
    room.area=0x22;room.room=0x15;room.width=240;room.height=160;
    room.map_x=320;room.map_y=96;
    static MapDataDefinition three;
    gMain.task=TASK_GAME;gMain.state=GAMETASK_MAIN;
    gMain.substate=GAMEMAIN_CHANGEROOM;
    gRoomControls.area=room.area;gRoomControls.room=room.room;
    gPlayerEntity.base.kind=PLAYER;gEntCount=1;gManagerCount=0;
    RoomResInfo* orig=&gArea.roomResInfos[room.room];
    gArea.pCurrentRoomInfo=orig;
    orig->map_x=room.map_x;orig->map_y=room.map_y;
    orig->pixel_width=room.width;orig->pixel_height=room.height;
    orig->tileSet=&three;orig->tiles=&three;orig->map=&three;
    assert(gRoomControls.origin_x==0 && gRoomControls.width==0);
    DsiRoomMetadata wrong=room;wrong.map_x++;
    assert(!DsiRoomGeometryE38_BindBeforeSpawn(&wrong));
    assert(gRoomControls.origin_x==0 && gRoomControls.width==0);
    assert(DsiRoomGeometryE38_Reason()==4);
    DsiRoomGeometryE38_Reset();
    gRoomControls.origin_y=1;
    assert(!DsiRoomGeometryE38_BindBeforeSpawn(&room));
    assert(gRoomControls.origin_x==0 && gRoomControls.width==0);
    assert(DsiRoomGeometryE38_Reason()==5);
    DsiRoomGeometryE38_Reset();gRoomControls.origin_y=0;
    assert(DsiRoomGeometryE38_BindBeforeSpawn(&room));
    assert(DsiRoomGeometryE38_Ready());
    assert(gRoomControls.origin_x==320 && gRoomControls.origin_y==96);
    assert(gRoomControls.width==240 && gRoomControls.height==160);
    /* The original sub_0804AF0C adds dat->xPos, dat->yPos to origin.
     * No gameplay actor positions are fabricated in the adapter. */
    assert(gRoomControls.origin_x+0x58==408);
    assert(gRoomControls.origin_y+8==104);
    assert(DsiRoomGeometryE38_BindBeforeSpawn(&room));
    gEntCount=2;
    assert(!DsiRoomGeometryE38_BindBeforeSpawn(&room));
    assert(DsiRoomGeometryE38_Reason()==2);
    DsiRoomGeometryE38_Reset();
    assert(!DsiRoomGeometryE38_Ready() && DsiRoomGeometryE38_Reason()==0);
    puts("E38 original InitRoom -> pre-spawn geometry: mismatches, guard, coordinates, reset PASS");
}
