#ifndef DSI_ROOM_RESOURCES_H
#define DSI_ROOM_RESOURCES_H
/* E27.2: persistent, native copies of the EU ROM room resource descriptors.
 * The originals are MapDataDefinition records with 32-bit GBA addresses.
 * Never cast a GBA ROM/RAM address to a C pointer on Nintendo DSi.
 * This is NOT an alternative room engine and contains no gameplay logic. */
#include <stdbool.h>
#include <stdint.h>
#include "dsi_room_probe.h"

#define DSI_ROOM_STREAM_MAX 16u
typedef struct {
    uint32_t src;  /* Original packed ROM offset / palette group */
    uint32_t dest; /* Original GBA destination, NOT a native pointer */
    uint32_t size; /* Original packed size/compression flag */
} DsiOriginalMapCommand;

typedef struct {
    uint32_t gba_address;
    uint8_t count;
    DsiOriginalMapCommand command[DSI_ROOM_STREAM_MAX];
} DsiOriginalMapStream;

typedef struct {
    bool valid;
    uint8_t area, room;
    uint16_t map_x, map_y, width, height, tileSet_id;
    uint32_t room_map_address;
    uint32_t tileset_address;
    uint32_t area_tiles_address;
    uint32_t properties_address;
    uint32_t entity_list_addresses[2];
    uint16_t entity_counts[2];
    /* Fixed native memory: records persist across further SD/ROM cache reads. */
    DsiOriginalMapStream map, tileset, tiles;
} DsiRoomResources;

void DsiRoomResources_Reset(void);
/* Atomic: failure does NOT corrupt or replace the last successful snapshot. */
bool DsiRoomResources_PrepareEU(const DsiRoomMetadata *room);
/* Stable until reset or the next successful PrepareEU. NULL when unavailable. */
const DsiRoomResources *DsiRoomResources_Current(void);
#endif
