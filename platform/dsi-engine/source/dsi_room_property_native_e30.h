#ifndef DSI_ROOM_PROPERTY_NATIVE_E30_H
#define DSI_ROOM_PROPERTY_NATIVE_E30_H
/* E30.3: attach ONLY original EU EntityData banks 0/1 to the ORIGINAL
 * RoomResInfo.properties pointer. NO GBA callbacks, scripts, NPC spawning,
 * input or original game-loop update is enabled by this adapter. */
#include <stdbool.h>
#include "dsi_room_probe.h"
void DsiRoomPropertyNativeE30_Reset(void);
bool DsiRoomPropertyNativeE30_Attach(const DsiRoomMetadata *room);
bool DsiRoomPropertyNativeE30_Ready(void);
#endif
