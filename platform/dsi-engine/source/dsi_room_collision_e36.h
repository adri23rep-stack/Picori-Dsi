#ifndef DSI_ROOM_COLLISION_E36_H
#define DSI_ROOM_COLLISION_E36_H
#include <stdbool.h>
#include "dsi_room_probe.h"
bool DsiRoomCollisionE36_Prepare(const DsiRoomMetadata *room);
bool DsiRoomCollisionE36_Ready(void);
unsigned DsiRoomCollisionE36_Errors(void);
#endif
