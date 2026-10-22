/* E36 regression compiles ORIGINAL CreateCollisionDataBorderAroundRoom
 * unchanged from src/playerUtils.c. No mocked wall/collision algorithm. */
#include "dsi_room_collision_e36.h"
#include "room.h"
#include "map.h"
#include <assert.h>
#include <stdio.h>
#include <string.h>
RoomControls gRoomControls;
MapLayer gMapBottom,gMapTop;
static bool roomReady;
bool DsiRoomSpawnE31_Ready(void) { return roomReady; }
int main(void) {
    DsiRoomMetadata r={0};
    r.area=0x22;r.room=0x15;r.width=240;r.height=160;
    r.map_x=64;r.map_y=32;
    gRoomControls.area=r.area;gRoomControls.room=r.room;
    gRoomControls.width=r.width;gRoomControls.height=r.height;
    gRoomControls.origin_x=r.map_x;gRoomControls.origin_y=r.map_y;
    gMapBottom.collisionData[0]=0x17;
    gMapTop.collisionData[0]=0x0a;
    assert(!DsiRoomCollisionE36_Prepare(&r));
    assert(gMapBottom.collisionData[15]==0);
    roomReady=true;
    DsiRoomMetadata bad=r;bad.area=0x21;
    assert(!DsiRoomCollisionE36_Prepare(&bad));
    bad=r;bad.width=1024;
    assert(!DsiRoomCollisionE36_Prepare(&bad));
    assert(DsiRoomCollisionE36_Prepare(&r));
    assert(DsiRoomCollisionE36_Ready());
    assert(DsiRoomCollisionE36_Errors()==0);
    for(unsigned i=0;i<64;i++){
      assert(gMapBottom.collisionData[i*64+15]==0xff);
      assert(gMapTop.collisionData[i*64+15]==0xff);
      assert(gMapBottom.collisionData[i*64+63]==0xff);
      assert(gMapTop.collisionData[i*64+63]==0xff);
      assert(gMapBottom.collisionData[10*64+i]==0xff);
      assert(gMapTop.collisionData[10*64+i]==0xff);
      assert(gMapBottom.collisionData[63*64+i]==0xff);
      assert(gMapTop.collisionData[63*64+i]==0xff);
    }
    assert(gMapBottom.collisionData[0]==0x17);
    assert(gMapTop.collisionData[0]==0x0a);
    assert(DsiRoomCollisionE36_Prepare(&r));
    assert(gMapBottom.collisionData[0]==0x17);
    puts("E36 original CreateCollisionDataBorderAroundRoom: bounds, originals, persistent layers PASS");
}
