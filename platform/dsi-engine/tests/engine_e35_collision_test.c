#include "dsi_collision_audit_e35.h"
#include "map.h"
#include "room.h"
#include "player.h"
#include <assert.h>
#include <stdio.h>
#include <string.h>
RoomControls gRoomControls;
MapLayer gMapBottom,gMapTop;
PlayerEntity gPlayerEntity;
static int room_ready;
bool DsiRoomSpawnE31_Ready(void){return room_ready!=0;}
extern u32 GetCollisionDataAtTilePos(u32,u32);
extern u32 GetCollisionDataAtWorldCoords(u32,u32,u32);
extern u32 GetCollisionDataAtEntity(Entity*);
extern u32 GetCollisionDataRelativeTo(Entity*,s32,s32);
int main(void){
    gRoomControls.origin_x=100;gRoomControls.origin_y=200;
    gPlayerEntity.base.kind=PLAYER;
    gPlayerEntity.base.collisionLayer=1;
    gPlayerEntity.base.x.HALF.HI=124;gPlayerEntity.base.y.HALF.HI=232;
    gMapBottom.collisionData[129]=0x0fu;
    gMapTop.collisionData[129]=0xffu;
    assert(!DsiCollisionAuditE35_Run(128,128));
    room_ready=1;
    assert(GetCollisionDataAtTilePos(129,1)==15);
    assert(GetCollisionDataAtTilePos(129,2)==255);
    assert(GetCollisionDataAtWorldCoords(124,232,1)==15);
    assert(GetCollisionDataAtEntity(&gPlayerEntity.base)==15);
    assert(GetCollisionDataRelativeTo(&gPlayerEntity.base,0,0)==15);
    assert(GetCollisionDataRelativeTo(&gPlayerEntity.base,16,0)==0);
    assert(DsiCollisionAuditE35_Run(128,128));
    assert(DsiCollisionAuditE35_NonzeroTiles()==2);
    memset(gMapBottom.collisionData,0,sizeof(gMapBottom.collisionData));
    memset(gMapTop.collisionData,0,sizeof(gMapTop.collisionData));
    assert(!DsiCollisionAuditE35_Run(128,128));
    assert(DsiCollisionAuditE35_NonzeroTiles()==0);
    puts("E35 original GBA layer collision data, read-only audit, empty-map gate PASS");
}
