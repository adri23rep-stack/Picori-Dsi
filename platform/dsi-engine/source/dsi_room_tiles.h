#ifndef DSI_ROOM_TILES_H
#define DSI_ROOM_TILES_H
/* E22: read the original EU room tileSet and tiles commands, bounded via
 * the same MapDataDefinition transfer adapter used for E21 maps. */
#include <stdbool.h>
#include "dsi_room_maps.h"
typedef struct {
    DsiRoomMapStats tileset, tiles;
} DsiRoomTileStats;
bool DsiRoomTiles_LoadEU(const DsiRoomMetadata *room, DsiRoomTileStats *stats);
#endif
