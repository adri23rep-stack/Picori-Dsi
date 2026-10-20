#ifndef DSI_E28_AREA_HEADERS_H
#define DSI_E28_AREA_HEADERS_H
/* Stable native RoomHeader mirror of the ORIGINAL EU ROM table.
 * No game logic lives here. On GBA each header is a ten-byte record. */
#include <stdbool.h>
#include <stdint.h>
#include "area.h"
#define DSI_E28_MAX_ROOMS MAX_ROOMS
typedef struct {
    bool ready;
    uint8_t area;
    uint8_t count;
    RoomHeader rooms[DSI_E28_MAX_ROOMS];
} DsiAreaHeaders;
void DsiAreaHeaders_Reset(void);
/* Failed preparation leaves the previous snapshot untouched. */
bool DsiAreaHeaders_PrepareEU(uint8_t area);
/* Stable pointer until next successful PrepareEU or Reset. */
const DsiAreaHeaders *DsiAreaHeaders_Current(void);
const RoomHeader *DsiAreaHeaders_Get(uint8_t area, uint8_t room);
#endif
