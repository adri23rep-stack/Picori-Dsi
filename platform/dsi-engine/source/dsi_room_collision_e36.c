/* E36: original room collision boundary, not recreated collision logic.
 * This checks native ROM-derived dimensions, then calls the actual
 * CreateCollisionDataBorderAroundRoom in existing src/playerUtils.c.
 * Link movement must remain disabled until original interior collision
 * data + player action/frame dispatch are fully ported. */
#include "dsi_room_collision_e36.h"
#include "dsi_room_spawn_e31.h"
#include "room.h"
#include "area.h"
#include "map.h"
#include <stddef.h>
#include <stdint.h>
static bool sReady;
static unsigned sErrors;
void DsiRoomCollisionE36_Reset(void) {sReady=false;sErrors=0;}
extern void CreateCollisionDataBorderAroundRoom(void);
bool DsiRoomCollisionE36_Ready(void) {
    return sReady && sErrors==0 && DsiRoomSpawnE31_Ready();
}
unsigned DsiRoomCollisionE36_Errors(void) { return sErrors; }
bool DsiRoomCollisionE36_Prepare(const DsiRoomMetadata *room) {
    if(sReady) return DsiRoomCollisionE36_Ready() && room &&
        room->area==gRoomControls.area && room->room==gRoomControls.room &&
        room->width==gRoomControls.width && room->height==gRoomControls.height &&
        room->map_x==gRoomControls.origin_x &&
        room->map_y==gRoomControls.origin_y;
    if (sErrors || !room || !DsiRoomSpawnE31_Ready() ||
        room->area!=0x22u || room->room!=0x15u ||
        room->width<16 || room->height<16 ||
        room->width>1008 || room->height>1008 ||
        (room->width&15u) || (room->height&15u) ||
        gRoomControls.area!=room->area ||
        gRoomControls.room!=room->room) return false;
    /* E37: original GameMain_InitRoom did not run InitializeCamera,
     * leaving width, height and origin uninitialized. Bind ONLY values
     * independently verified in the REAL original InitRoom RoomResInfo.
     * Never invent room geometry or ignore stale conflicting fields. */
    const RoomResInfo *original = gArea.pCurrentRoomInfo;
    if(!original ||
       original != &gArea.roomResInfos[room->room] ||
       original->pixel_width!=room->width ||
       original->pixel_height!=room->height ||
       original->map_x!=room->map_x ||
       original->map_y!=room->map_y ||
       (gRoomControls.width && gRoomControls.width!=room->width) ||
       (gRoomControls.height && gRoomControls.height!=room->height) ||
       (gRoomControls.origin_x && gRoomControls.origin_x!=room->map_x) ||
       (gRoomControls.origin_y && gRoomControls.origin_y!=room->map_y))
        return false;
    const unsigned col=room->width/16u;
    const unsigned row=room->height/16u;
    if(col>=64u || row>=64u) return false;
    gRoomControls.width=original->pixel_width;
    gRoomControls.height=original->pixel_height;
    gRoomControls.origin_x=original->map_x;
    gRoomControls.origin_y=original->map_y;
    /* The original 64x64 MapLayers are owned by E31.
     * Now run the UNMODIFIED original source collision boundary. */
    CreateCollisionDataBorderAroundRoom();
    for(unsigned i=0;i<64u;i++) {
        if(gMapBottom.collisionData[i*64u+col]!=0xffu ||
           gMapBottom.collisionData[i*64u+63u]!=0xffu ||
           gMapTop.collisionData[i*64u+col]!=0xffu ||
           gMapTop.collisionData[i*64u+63u]!=0xffu ||
           gMapBottom.collisionData[row*64u+i]!=0xffu ||
           gMapBottom.collisionData[63u*64u+i]!=0xffu ||
           gMapTop.collisionData[row*64u+i]!=0xffu ||
           gMapTop.collisionData[63u*64u+i]!=0xffu) {
            ++sErrors; return false;
        }
    }
    sReady=true;
    return true;
}
