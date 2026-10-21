#ifndef DSI_ROOM_PROPERTIES_E30_H
#define DSI_ROOM_PROPERTIES_E30_H
/* A native, READ-ONLY snapshot of the original EU GBA room property table.
 * The original entries are 32-bit cartridge addresses, not ARM9 C pointers.
 * No room scripts or game logic are reimplemented or dispatched here. */
#include <stdbool.h>
#include <stdint.h>
#include "dsi_room_probe.h"

#define DSI_E30_PROPERTY_SLOTS 8u
enum {
    DSI_E30_PROPERTY_NONE = 0u,
    DSI_E30_PROPERTY_ROM_DATA = 1u,
    DSI_E30_PROPERTY_THUMB_ADDRESS = 2u
};
typedef struct {
    bool valid;
    uint8_t area, room;
    uint32_t gba_table_address;
    uint32_t original[DSI_E30_PROPERTY_SLOTS];
    uint8_t kind[DSI_E30_PROPERTY_SLOTS];
    /* Never set to true before original callbacks and property consumers
     * are actually ported and enabled. A data snapshot is NOT execution. */
    bool native_dispatch_ready;
} DsiRoomPropertiesE30;
void DsiRoomPropertiesE30_Reset(void);
/* Reject malformed/unreadable pointers and preserve the prior snapshot on
 * failure. 0/1 must match the verified original entity-list addresses. */
bool DsiRoomPropertiesE30_PrepareEU(const DsiRoomMetadata *room);
const DsiRoomPropertiesE30 *DsiRoomPropertiesE30_Current(void);
#endif
