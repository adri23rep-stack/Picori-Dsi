/* E30.3: bridge verified original EntityData records to RoomResInfo.
 * The records are original EU ROM data copied into native ARM9 RAM.
 * Their embedded GBA addresses (including spritePtr) are NOT dereferenced.
 * Slots 2..7 are intentionally NULL, NEVER native code pointers.
 */
#include "dsi_room_property_native_e30.h"
#include "dsi_room_property_data_e30.h"
#include "dsi_room_properties_e30.h"
#include "area.h"
#include "room.h"
#include <stddef.h>
#include <stdint.h>

static void *sNativeSlots[DSI_E30_PROPERTY_SLOTS];
static RoomResInfo *sRoom;
static uint8_t sArea, sRoomId;
static bool sAttached;

void DsiRoomPropertyNativeE30_Reset(void) {
    /* Only detach a pointer we actually own in the original room struct. */
    if (sRoom && sRoom->properties == sNativeSlots)
        sRoom->properties = NULL;
    sRoom = NULL;
    sAttached = false;
    sArea = sRoomId = 0;
    for (unsigned i=0;i<DSI_E30_PROPERTY_SLOTS;++i) sNativeSlots[i] = NULL;
}

bool DsiRoomPropertyNativeE30_Ready(void) {
    if (!sAttached || !sRoom ||
        gRoomControls.area != sArea || gRoomControls.room != sRoomId ||
        sRoom != &gArea.roomResInfos[sRoomId] ||
        gArea.pCurrentRoomInfo != sRoom ||
        sRoom->properties != sNativeSlots ||
        !DsiRoomPropertyDataE30_Ready())
        return false;
    const DsiRoomPropertiesE30 *p = DsiRoomPropertiesE30_Current();
    if (!p || !p->valid || p->area != sArea || p->room != sRoomId ||
        p->native_dispatch_ready) return false;
    for (unsigned i=0;i<DSI_E30_PROPERTY_SLOTS;++i) {
        void *want = (i < DSI_ENTITY_LIST_BANKS)
            ? (void*)DsiRoomPropertyDataE30_EntityBank(i) : NULL;
        if (sNativeSlots[i] != want) return false;
    }
    return true;
}

bool DsiRoomPropertyNativeE30_Attach(const DsiRoomMetadata *room) {
    if (!room || room->area >= 0x90u || room->room >= MAX_ROOMS ||
        room->area != gRoomControls.area || room->room != gRoomControls.room ||
        !DsiRoomPropertyDataE30_Ready() ||
        sAttached || sRoom)
        return false;
    const DsiRoomPropertiesE30 *p = DsiRoomPropertiesE30_Current();
    RoomResInfo *info = &gArea.roomResInfos[room->room];
    if (!p || !p->valid || p->area != room->area || p->room != room->room ||
        p->native_dispatch_ready ||
        gArea.pCurrentRoomInfo != info ||
        info->properties != NULL ||
        info->pixel_width != room->width || info->pixel_height != room->height ||
        info->map_x != room->map_x || info->map_y != room->map_y ||
        !info->tileSet || !info->map || !info->tiles)
        return false;
    void *bank0=(void*)DsiRoomPropertyDataE30_EntityBank(0);
    void *bank1=(void*)DsiRoomPropertyDataE30_EntityBank(1);
    if (!bank0 || !bank1) return false;
    for (unsigned i=0;i<DSI_E30_PROPERTY_SLOTS;++i)
        sNativeSlots[i] = NULL;
    sNativeSlots[0]=bank0;
    sNativeSlots[1]=bank1;
    sRoom=info;
    sArea=room->area;
    sRoomId=room->room;
    sAttached=true;
    info->properties=sNativeSlots; /* true RoomResInfo field, not a clone */
    return DsiRoomPropertyNativeE30_Ready();
}
