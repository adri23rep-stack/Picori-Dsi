/* E38: missing step in the ORIGINAL GameMain_ChangeRoom path.
 * The skipped InitializeCamera normally establishes RoomControls geometry
 * BEFORE the original LoadRoomEntityList calls sub_0804AF0C:
 *     ent->x = dat->xPos + gRoomControls.origin_x
 *     ent->y = dat->yPos + gRoomControls.origin_y
 * E31 was spawning in a zero-origin temporary room and then E37 set origin
 * afterwards, which is too late for the genuine object positions.
 * Use ONLY the original InitRoom RoomResInfo and independently-read EU ROM.
 * This is a state-binding bridge, not a replacement world/physics implementation.
 */
#include "dsi_room_geometry_e38.h"
#include "room.h"
#include "area.h"
#include "game.h"
#include "main.h"
#include "player.h"
#include "entity.h"
#include <stdint.h>
static bool sReady;
static unsigned sReason;
void DsiRoomGeometryE38_Reset(void) {sReady=false;sReason=0;}
bool DsiRoomGeometryE38_Ready(void) {return sReady && !sReason;}
unsigned DsiRoomGeometryE38_Reason(void) {return sReason;}
bool DsiRoomGeometryE38_BindBeforeSpawn(const DsiRoomMetadata *room) {
    if(!room || room->area!=0x22u || room->room!=0x15u ||
       !room->width || !room->height || room->width>1008u ||
       room->height>1008u || (room->width&15u) || (room->height&15u)) {
        sReason=1;return false;
    }
    if(gMain.task!=TASK_GAME || gMain.state!=GAMETASK_MAIN ||
       gMain.substate!=GAMEMAIN_CHANGEROOM ||
       gRoomControls.area!=room->area || gRoomControls.room!=room->room ||
       gEntCount!=1u || gManagerCount!=0u ||
       gPlayerEntity.base.kind!=PLAYER) {
        sReason=2;return false;
    }
    if(!gArea.pCurrentRoomInfo ||
       gArea.pCurrentRoomInfo!=&gArea.roomResInfos[room->room]) {
        sReason=3;return false;
    }
    const RoomResInfo *source=gArea.pCurrentRoomInfo;
    if(source->map_x!=room->map_x || source->map_y!=room->map_y ||
       source->pixel_width!=room->width ||
       source->pixel_height!=room->height ||
       !source->tileSet || !source->map || !source->tiles) {
        sReason=4;return false;
    }
    if((gRoomControls.origin_x && gRoomControls.origin_x!=room->map_x) ||
       (gRoomControls.origin_y && gRoomControls.origin_y!=room->map_y) ||
       (gRoomControls.width && gRoomControls.width!=room->width) ||
       (gRoomControls.height && gRoomControls.height!=room->height)) {
        sReason=5;return false;
    }
    gRoomControls.origin_x=source->map_x;
    gRoomControls.origin_y=source->map_y;
    gRoomControls.width=source->pixel_width;
    gRoomControls.height=source->pixel_height;
    sReady=true;sReason=0;
    return true;
}
