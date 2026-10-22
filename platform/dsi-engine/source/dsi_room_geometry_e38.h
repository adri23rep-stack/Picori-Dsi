#ifndef DSI_ROOM_GEOMETRY_E38_H
#define DSI_ROOM_GEOMETRY_E38_H
#include <stdbool.h>
#include "dsi_room_probe.h"
/* Bind original RoomResInfo coordinates BEFORE original LoadRoomEntityList. */
void DsiRoomGeometryE38_Reset(void);
bool DsiRoomGeometryE38_BindBeforeSpawn(const DsiRoomMetadata *room);
bool DsiRoomGeometryE38_Ready(void);
unsigned DsiRoomGeometryE38_Reason(void);
#endif
