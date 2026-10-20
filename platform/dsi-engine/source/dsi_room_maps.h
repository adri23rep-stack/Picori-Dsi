#ifndef DSI_ROOM_MAPS_H
#define DSI_ROOM_MAPS_H
/* ROM MapDataDefinition bridge: reproduces original data transfers, not gameplay. */
#include <stdbool.h>
#include <stdint.h>
#include "dsi_room_probe.h"
typedef struct {
    uint32_t descriptors;
    uint32_t uncompressed_bytes;
    uint32_t lz_blocks;
    uint32_t palette_groups;
} DsiRoomMapStats;
/* Load the room's ORIGINAL MapDataDefinition list to GBA shadow RAM.
 * Fail closed on unsupported commands, bad ROM offsets or destinations.
 * The caller must attach a verified European retail ROM to DsiGbaBus.
 * This does not initialize the room or render its metatiles. */
bool DsiRoomMaps_LoadEU(const DsiRoomMetadata *room, DsiRoomMapStats *stats);
#endif
