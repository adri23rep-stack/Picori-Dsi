#ifndef DSI_ROOM_COLLISION_E41_H
#define DSI_ROOM_COLLISION_E41_H
#include "dsi_room_probe.h"
#define DSI_E41_TILE_TYPES 1396u
#define DSI_E41_SPECIAL_TILES 151u
bool DsiRoomCollisionE41_Prepare(const DsiRoomMetadata* room);
bool DsiRoomCollisionE41_Ready(void);
void DsiRoomCollisionE41_Reset(void);
unsigned DsiRoomCollisionE41_Errors(void);
#endif
