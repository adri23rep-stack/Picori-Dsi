/* Integration: unchanged original InitRoom -> InitAllRoomResInfo ->
 * InitRoomResInfo -> GetCurrentRoomInfo; ROM and fade mocks stay in host test. */
#include "area.h"
#include "game.h"
#include "flags.h"
#include "room.h"
#include "fade.h"
#include "dsi_area_headers.h"
#include "dsi_room_resources.h"
#include "common.h"
#include <assert.h>
#include <stdio.h>
#include <string.h>
void InitRoom(void);
Area gArea;
RoomControls gRoomControls;
RoomTransition gRoomTransition;
static DsiAreaHeaders source_headers;
static DsiRoomResources source_resources;
static unsigned fades;
void MemClear(void *out, u32 len) { memset(out, 0, len); }
void SetFade(u32 type,u32 speed) {
    assert((type & FADE_MOSAIC) != 0);
    assert(speed == 8);
    ++fades;
}
void SetFadeInverted(u32 speed) {
    assert(speed==16);
    ++fades;
}
const DsiAreaHeaders *DsiAreaHeaders_Current(void) {
    return source_headers.ready ? &source_headers : NULL;
}
const DsiRoomResources *DsiRoomResources_Current(void) {
    return source_resources.valid ? &source_resources : NULL;
}
RoomHeader *GetAreaRoomHeaderTable(u32 area) {
    const DsiAreaHeaders *s = DsiAreaHeaders_Current();
    return s && s->area == area ? (RoomHeader*)(void*)s->rooms : NULL;
}
int main(void) {
    source_headers.ready = true;
    source_headers.area = AREA_HOUSE_INTERIORS_2;
    source_headers.count = 2;
    source_headers.rooms[0] = (RoomHeader){0, 0, 128, 128, 2};
    source_headers.rooms[1] = (RoomHeader){160, 48, 240, 160, 7};
    source_headers.rooms[2].map_x = 0xFFFFu;
    source_resources.valid = true;
    source_resources.area = source_headers.area;
    source_resources.room = 1;
    source_resources.map.command[0] = (DsiOriginalMapCommand){0x20, 0x02000100, 32};
    source_resources.tileset.command[0] = (DsiOriginalMapCommand){0x30, 0x02000200, 64};
    source_resources.tiles.command[0] = (DsiOriginalMapCommand){0x40, 0x02000300, 64};
    gRoomControls.area = source_headers.area;
    gRoomControls.room = 1;
    gRoomTransition.type = TRANSITION_CUT;
    memset(&gArea,0xCD,sizeof gArea);
    InitRoom();
    assert(fades == 1);
    assert(gArea.flag_bank == gAreaMetadata[source_headers.area].flag_bank);
    assert(gArea.localFlagOffset == gLocalFlagBanks[gArea.flag_bank]);
    assert(gArea.portal_timer == 180 && gArea.lightLevel == 256);
    assert(gArea.pCurrentRoomInfo == &gArea.roomResInfos[1]);
    const RoomResInfo *active = gArea.pCurrentRoomInfo;
    assert(active->map_x == 160 && active->map_y == 48);
    assert(active->pixel_width == 240 && active->pixel_height == 160);
    assert(active->map == (MapDataDefinition*)(void*)source_resources.map.command);
    assert(active->tileSet == (MapDataDefinition*)(void*)source_resources.tileset.command);
    assert(active->tiles == (MapDataDefinition*)(void*)source_resources.tiles.command);
    assert(active->properties == NULL && active->exits == NULL && active->bg_anim == NULL);
    assert(gArea.roomResInfos[0].map == NULL);
    assert(gArea.roomResInfos[2].pixel_width == 0);
    source_headers.ready = false;
    InitRoom();
    assert(gArea.pCurrentRoomInfo == &gArea.roomResInfos[1]);
    assert(gArea.roomResInfos[1].map == NULL);
    puts("PASS E28.4: genuine full InitRoom chain and native RoomResInfo pointers");
    return 0;
}
