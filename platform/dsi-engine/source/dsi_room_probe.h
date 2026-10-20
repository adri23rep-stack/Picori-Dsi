#ifndef DSI_ROOM_PROBE_H
#define DSI_ROOM_PROBE_H
/* Bounded, read-only ROM room-metadata bridge. No simulation of gameplay. */
#include <stdbool.h>
#include <stdint.h>

typedef struct {
    uint8_t area, room;
    uint16_t map_x, map_y, width, height, tileset_id;
    uint32_t room_map_address;
    uint32_t properties_address;
    uint32_t entity_list_addresses[2];
    uint16_t entity_counts[2];
} DsiRoomMetadata;

/* EU retail tables, per kRomOffsets_EU in port/port_rom.c.
 * Caller must attach a verified European ROM to DsiGbaBus first.
 * Does not initialize room, spawn entities, or write save files. */
bool DsiRoomProbe_ReadEU(uint8_t area, uint8_t room, DsiRoomMetadata *out);
#endif
