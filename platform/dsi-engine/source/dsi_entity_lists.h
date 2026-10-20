#ifndef DSI_E28_ENTITY_LISTS_H
#define DSI_E28_ENTITY_LISTS_H
/* A byte-accurate, persistent snapshot of the ORIGINAL EU ROM entity lists.
 * No spawning, animation, Link implementation, or game logic lives here.
 * Future src/entity.c consumers will resolve native EntityData pointers. */
#include <stdbool.h>
#include <stdint.h>
#include "dsi_room_probe.h"

#define DSI_ENTITY_LIST_BANKS 2u
#define DSI_ENTITY_RECORD_BYTES 16u
#define DSI_ENTITY_RECORD_MAX 128u
typedef struct {
    bool valid;
    uint8_t area, room;
    uint16_t count[DSI_ENTITY_LIST_BANKS];
    uint32_t gba_address[DSI_ENTITY_LIST_BANKS];
    /* Every record and its original 0xFF list terminator are preserved. */
    uint8_t raw[DSI_ENTITY_LIST_BANKS][DSI_ENTITY_RECORD_MAX + 1u][DSI_ENTITY_RECORD_BYTES];
} DsiEntityLists;
void DsiEntityLists_Reset(void);
bool DsiEntityLists_PrepareEU(const DsiRoomMetadata *room);
const DsiEntityLists *DsiEntityLists_Current(void);
#endif
