/* E39 proof: saved-status positions are ROOM-LOCAL (src/gameUtils.c
 * sub_08053250), and actual InitRoom header moves them into world space.
 * No fake Link movement or animation, just pre-InitializePlayer state. */
#include "dsi_room_link_origin_e39.h"
#include "room.h"
#include "area.h"
#include "game.h"
#include "main.h"
#include <assert.h>
#include <stdio.h>
RoomTransition gRoomTransition;
RoomControls gRoomControls;
Area gArea;
Main gMain;
int main(void) {
    DsiRoomMetadata d={0};
    d.area=0x22;d.room=0x15;d.map_x=0x510;d.map_y=0x1b0;
    d.width=240;d.height=160;
    gRoomControls.area=d.area;gRoomControls.room=d.room;
    gArea.pCurrentRoomInfo=&gArea.roomResInfos[d.room];
    gArea.pCurrentRoomInfo->map_x=d.map_x;
    gArea.pCurrentRoomInfo->map_y=d.map_y;
    gArea.pCurrentRoomInfo->pixel_width=d.width;
    gArea.pCurrentRoomInfo->pixel_height=d.height;
    gMain.task=TASK_GAME;gMain.state=GAMETASK_MAIN;
    gMain.substate=GAMEMAIN_CHANGEROOM;
    gRoomTransition.player_status.start_pos_x=0x90;
    gRoomTransition.player_status.start_pos_y=0x38;
    assert(DsiRoomLinkOriginE39_BindBeforeInitializePlayer(&d));
    assert(DsiRoomLinkOriginE39_Ready());
    assert(gRoomTransition.player_status.start_pos_x==0x5a0);
    assert(gRoomTransition.player_status.start_pos_y==0x1e8);
    assert(!DsiRoomLinkOriginE39_BindBeforeInitializePlayer(&d));
    DsiRoomLinkOriginE39_Reset();
    assert(!DsiRoomLinkOriginE39_BindBeforeInitializePlayer(&d));
    assert(DsiRoomLinkOriginE39_Reason()==4);
    DsiRoomLinkOriginE39_Reset();
    gRoomTransition.player_status.start_pos_x=0x90;
    gRoomTransition.player_status.start_pos_y=0x38;
    DsiRoomMetadata wrong=d;wrong.map_x=0x500;
    assert(!DsiRoomLinkOriginE39_BindBeforeInitializePlayer(&wrong));
    assert(gRoomTransition.player_status.start_pos_x==0x90);
    DsiRoomLinkOriginE39_Reset();
    assert(DsiRoomLinkOriginE39_BindBeforeInitializePlayer(&d));
    puts("E39 original save local->world coordinates and strict guards PASS");
}
