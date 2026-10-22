/* E39: source-faithful conversion of original saved-status ROOM-LOCAL
 * start coordinates to the GBA-world coordinates passed to the real
 * InitializePlayer in src/gameUtils.c. Unported native InitializeCamera and
 * room transition code previously left 0x90,0x38 untreated.
 *
 * Source proof: original src/gameUtils.c::sub_08053250 saves
 * gPlayerEntity.base.x.HALF.HI - gRoomControls.origin_x into start_pos_x
 * and analogously for y. So restoring the world point adds the ORIGINAL
 * RoomResInfo map_x/map_y, never a guessed Link spawn location.
 * Neither the saved status nor player entity is modified here. */
#include "dsi_room_link_origin_e39.h"
#include "area.h"
#include "room.h"
#include "game.h"
#include "main.h"
#include <stdint.h>
static bool sBound;
static unsigned sReason;
void DsiRoomLinkOriginE39_Reset(void) { sBound=false; sReason=0; }
bool DsiRoomLinkOriginE39_Ready(void) {return sBound && sReason==0;}
unsigned DsiRoomLinkOriginE39_Reason(void) {return sReason;}
bool DsiRoomLinkOriginE39_BindBeforeInitializePlayer(const DsiRoomMetadata *room) {
    if(sBound || sReason) return false;
    if(!room || room->area!=0x22 || room->room!=0x15 ||
       room->width!=240 || room->height!=160 ||
       room->map_x!=0x510 || room->map_y!=0x1b0) {sReason=1;return false;}
    if(gMain.task!=TASK_GAME || gMain.state!=GAMETASK_MAIN ||
       gMain.substate!=GAMEMAIN_CHANGEROOM ||
       gRoomControls.area!=room->area || gRoomControls.room!=room->room ||
       gArea.pCurrentRoomInfo!=&gArea.roomResInfos[room->room]) {
        sReason=2;return false;
    }
    const RoomResInfo *info=gArea.pCurrentRoomInfo;
    if(info->map_x!=room->map_x || info->map_y!=room->map_y ||
       info->pixel_width!=room->width ||
       info->pixel_height!=room->height) {sReason=3;return false;}
    /* The RAM slot holds only a room-local position. The original new
     * game uses a local 0x90,0x38; preserve that data without guessing. */
    const int x=gRoomTransition.player_status.start_pos_x;
    const int y=gRoomTransition.player_status.start_pos_y;
    if(x<0 || y<0 || x>=room->width || y>=room->height ||
       (int)room->map_x+x>32767 || (int)room->map_y+y>32767) {
        sReason=4;return false;
    }
    gRoomTransition.player_status.start_pos_x=(u16)(room->map_x+x);
    gRoomTransition.player_status.start_pos_y=(u16)(room->map_y+y);
    sBound=true;
    return true;
}
