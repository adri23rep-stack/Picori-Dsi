/* Host-only test doubles at HARDWARE/ROOM-RESOURCE boundaries.
 * The InitRoom/InitRoomTransition implementation is extracted unchanged
 * from the original gameUtils.c during compilation. */
#include "area.h"
#include "flags.h"
#include "game.h"
#include "room.h"
#include "fade.h"
#include "common.h"
#include <assert.h>
#include <string.h>
#include <stdio.h>
void InitRoomTransition(void);
Area gArea;
RoomControls gRoomControls;
RoomTransition gRoomTransition;
static unsigned info_calls, fades, inverted;
static u32 fade_type, fade_speed;
void MemClear(void* dest, u32 size) { memset(dest, 0, size); }
void InitAllRoomResInfo(void) { ++info_calls; }
void SetFade(u32 type, u32 speed) {
    ++fades;
    fade_type=type;
    fade_speed=speed;
}
void SetFadeInverted(u32 speed) { ++inverted; fade_speed=speed; }
int main(void) {
    gRoomControls.area=AREA_HOUSE_INTERIORS_2;
    gRoomTransition.type=TRANSITION_FADE_BLACK_SLOW;
    memset(&gArea,0xCC,sizeof gArea);
    InitRoom();
    assert(gArea.areaMetadata==gAreaMetadata[AREA_HOUSE_INTERIORS_2].flags);
    assert(gArea.flag_bank==gAreaMetadata[AREA_HOUSE_INTERIORS_2].flag_bank);
    assert(gArea.localFlagOffset==gLocalFlagBanks[gArea.flag_bank]);
    assert(gArea.portal_timer==180 && gArea.lightLevel==256);
    assert(info_calls==1 && inverted==1 && fade_speed==16);
    gRoomTransition.type=TRANSITION_CUT;
    InitRoomTransition();
    assert(fades==1 && fade_speed==8 && (fade_type&FADE_MOSAIC));
    /* Invalid area is rejected by the original PC_PORT region guard. */
    gRoomControls.area=255;
    InitRoom();
    assert(gRoomControls.area==0);
    assert(info_calls==2);
    puts("PASS E28: original InitRoom metadata, fade transition and resource boundary");
    return 0;
}
