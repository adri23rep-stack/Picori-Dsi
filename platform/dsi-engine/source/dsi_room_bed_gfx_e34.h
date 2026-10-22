#ifndef DSI_ROOM_BED_GFX_E34_H
#define DSI_ROOM_BED_GFX_E34_H
#include <stdbool.h>
#include <stddef.h>
#include <stdint.h>
#include "dsi_room_probe.h"
/* Preview ONLY: ROM-origin tile/OAM transport, no alternate Zelda AI. */
bool DsiRoomBedGfxE34_Draw(const DsiRoomMetadata*, uint16_t* out, size_t stride);
unsigned DsiRoomBedGfxE34_Errors(void);
#endif
