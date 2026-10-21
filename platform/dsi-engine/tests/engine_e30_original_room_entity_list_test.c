/* E30.4: original src/room.c LoadRoomEntityList reads BOTH native
 * E30.3 RoomResInfo entity property arrays. The only host mock is the
 * LoadRoomEntity per-record spawn boundary; real DSi must port it first.
 */
#include "dsi_room_property_native_e30.h"
#include "dsi_room_property_data_e30.h"
#include "dsi_room_properties_e30.h"
#include "area.h"
#include "room.h"
#include <assert.h>
#include <stdint.h>
#include <stdio.h>
#include <string.h>
Area gArea;
RoomControls gRoomControls;
static DsiRoomPropertiesE30 properties;
static EntityData data[2][4];
static bool banksReady;
static unsigned seen, resolutions, indices[8];
static const EntityData* accepted[2];
bool DsiRoomPropertyDataE30_Ready(void) { return banksReady; }
const EntityData* DsiRoomPropertyDataE30_EntityBank(unsigned b) {
    return banksReady && b < 2 ? data[b] : NULL;
}
const DsiRoomPropertiesE30* DsiRoomPropertiesE30_Current(void) {
    return properties.valid ? &properties : NULL;
}
/* The PC_PORT original resolver is isolated to known C pointers in this
 * unit test, not a dummy ROM resolver shipped into the DSi build. */
const void* Port_ResolveRegionData(const void* p) {
    ++resolutions;
    if (p == NULL || p == (const void*)data[0] ||
        p == (const void*)data[1]) return p;
    return NULL;
}
Entity* LoadRoomEntity(const EntityData *record) {
    /* Test-only observer of the original traversal; no fake Zelda entity. */
    assert(seen < 8);
    unsigned b = record >= data[1] && record < data[1]+4 ? 1 : 0;
    const EntityData* base = data[b];
    assert(record >= base && record < base + (b ? 2 : 3));
    indices[seen++] = ((unsigned)b << 8) | (unsigned)(record - base);
    return NULL;
}
static DsiRoomMetadata setup(void) {
    DsiRoomMetadata room={0};
    room.area=0x22; room.room=0x15;
    room.width=240; room.height=160;
    room.map_x=2; room.map_y=3;
    gRoomControls.area=room.area; gRoomControls.room=room.room;
    gArea.pCurrentRoomInfo=&gArea.roomResInfos[room.room];
    RoomResInfo *r=gArea.pCurrentRoomInfo;
    r->pixel_width=room.width; r->pixel_height=room.height;
    r->map_x=room.map_x; r->map_y=room.map_y;
    r->tileSet=(void*)data; r->map=(void*)data; r->tiles=(void*)data;
    properties.valid=true; properties.area=room.area; properties.room=room.room;
    data[0][0].kind=3; data[0][0].id=11;
    data[0][1].kind=6; data[0][1].id=12;
    data[0][2].kind=9; data[0][2].id=13;
    data[0][3].kind=0xFF;
    data[1][0].kind=7; data[1][0].id=21;
    data[1][1].kind=0xFF;
    banksReady=true;
    return room;
}
int main(void) {
    DsiRoomPropertyNativeE30_Reset();
    assert(!DsiRoomPropertyNativeE30_Ready());
    DsiRoomMetadata m=setup();
    assert(DsiRoomPropertyNativeE30_Attach(&m));
    assert(DsiRoomPropertyNativeE30_Ready());
    void** actual=gArea.pCurrentRoomInfo->properties;
    accepted[0]=(const EntityData*)actual[0];
    accepted[1]=(const EntityData*)actual[1];
    assert(accepted[0]==data[0] && accepted[1]==data[1]);
    LoadRoomEntityList(accepted[0]);
    LoadRoomEntityList(accepted[1]);
    LoadRoomEntityList(NULL);
    assert(seen==4);
    assert(indices[0]==0 && indices[1]==1 && indices[2]==2);
    assert(indices[3]==0x100);
    assert(resolutions==3); /* original PC_PORT resolver once per bank */
    assert(!properties.native_dispatch_ready);
    assert(gArea.pCurrentRoomInfo->properties[2]==NULL);
    DsiRoomPropertyNativeE30_Reset();
    assert(gArea.pCurrentRoomInfo->properties==NULL);
    puts("PASS E30.4: original LoadRoomEntityList traverses both genuine RoomResInfo banks");
    return 0;
}
