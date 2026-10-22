#ifndef DSI_ROOM_LINK_E35_H
#define DSI_ROOM_LINK_E35_H
#include <stdbool.h>
#include <stddef.h>
#include <stdint.h>
#include "dsi_room_probe.h"
#include "tmc_rom_stream.h"
bool DsiRoomLinkE35_Draw(const DsiRoomMetadata *room,TmcRomReader *rom,
                         uint16_t *out,size_t stride);
unsigned DsiRoomLinkE35_Errors(void);
unsigned DsiRoomLinkE35_Reason(void);
#endif
