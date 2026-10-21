/* E30.3 host-only test: original RoomResInfo field and native data banks.
 * Only simulated input data; NEVER a substitute Nintendo DS game routine. */
#include "dsi_room_property_native_e30.h"
#include "dsi_room_property_data_e30.h"
#include "dsi_room_properties_e30.h"
#include "area.h"
#include "room.h"
#include <assert.h>
#include <stdio.h>
#include <string.h>

Area gArea;
RoomControls gRoomControls;
static DsiRoomPropertiesE30 original;
static EntityData banks[DSI_ENTITY_LIST_BANKS][2];
static bool dataReady;

bool DsiRoomPropertyDataE30_Ready(void) { return dataReady; }
const EntityData *DsiRoomPropertyDataE30_EntityBank(unsigned bank) {
    return dataReady && bank < DSI_ENTITY_LIST_BANKS ? banks[bank] : NULL;
}
const DsiRoomPropertiesE30 *DsiRoomPropertiesE30_Current(void) {
    return original.valid ? &original : NULL;
}
static DsiRoomMetadata metadata(void) {
    DsiRoomMetadata m={0};
    m.area=0x22; m.room=0x15;
    m.width=240; m.height=160; m.map_x=2; m.map_y=3;
    return m;
}
static void setup(void) {
    memset(&gArea,0,sizeof gArea);
    memset(&gRoomControls,0,sizeof gRoomControls);
    memset(&original,0,sizeof original);
    dataReady=false;
    DsiRoomPropertyNativeE30_Reset();
    DsiRoomMetadata m=metadata();
    gRoomControls.area=m.area; gRoomControls.room=m.room;
    gArea.pCurrentRoomInfo=&gArea.roomResInfos[m.room];
    RoomResInfo *r=gArea.pCurrentRoomInfo;
    r->pixel_width=m.width; r->pixel_height=m.height;
    r->map_x=m.map_x; r->map_y=m.map_y;
    r->tileSet=(void*)banks;
    r->map=(void*)banks;
    r->tiles=(void*)banks;
    original.valid=true; original.area=m.area; original.room=m.room;
}
int main(void) {
    DsiRoomMetadata m=metadata();
    DsiRoomPropertyNativeE30_Reset();
    assert(!DsiRoomPropertyNativeE30_Ready());
    setup();
    assert(!DsiRoomPropertyNativeE30_Attach(&m));
    dataReady=true;
    assert(DsiRoomPropertyNativeE30_Attach(&m));
    assert(DsiRoomPropertyNativeE30_Ready());
    void **slots=gArea.pCurrentRoomInfo->properties;
    assert(slots && slots[0]==(void*)banks[0] && slots[1]==(void*)banks[1]);
    for (unsigned i=2;i<DSI_E30_PROPERTY_SLOTS;++i) assert(slots[i]==NULL);
    assert(!DsiRoomPropertyNativeE30_Attach(&m));
    assert(DsiRoomPropertyNativeE30_Ready());
    /* Never dispatch GBA ROM Thumb callbacks. */
    original.native_dispatch_ready=true;
    assert(!DsiRoomPropertyNativeE30_Ready());
    original.native_dispatch_ready=false;
    assert(DsiRoomPropertyNativeE30_Ready());
    dataReady=false;
    assert(!DsiRoomPropertyNativeE30_Ready());
    dataReady=true;
    assert(DsiRoomPropertyNativeE30_Ready());
    gRoomControls.room=0x16;
    assert(!DsiRoomPropertyNativeE30_Ready());
    gRoomControls.room=0x15;
    assert(DsiRoomPropertyNativeE30_Ready());
    slots[3]=(void*)banks[0]; /* corruption / illicit extra callback */
    assert(!DsiRoomPropertyNativeE30_Ready());
    slots[3]=NULL;
    assert(DsiRoomPropertyNativeE30_Ready());
    DsiRoomPropertyNativeE30_Reset();
    assert(gArea.pCurrentRoomInfo->properties==NULL);
    assert(!DsiRoomPropertyNativeE30_Ready());
    setup();
    gArea.pCurrentRoomInfo->properties=(void*)banks; /* real owner */
    dataReady=true;
    assert(!DsiRoomPropertyNativeE30_Attach(&m));
    assert(gArea.pCurrentRoomInfo->properties==(void*)banks);
    gArea.pCurrentRoomInfo->properties=NULL;
    gArea.pCurrentRoomInfo->pixel_width=256;
    assert(!DsiRoomPropertyNativeE30_Attach(&m));
    puts("PASS E30.3: original RoomResInfo owns safe native entity bank properties; callbacks remain unbound");
    return 0;
}
