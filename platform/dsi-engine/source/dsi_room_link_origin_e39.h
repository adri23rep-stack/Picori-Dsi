#ifndef DSI_ROOM_LINK_ORIGIN_E39_H
#define DSI_ROOM_LINK_ORIGIN_E39_H
#include <stdbool.h>
#include "dsi_room_probe.h"
void DsiRoomLinkOriginE39_Reset(void);
bool DsiRoomLinkOriginE39_BindBeforeInitializePlayer(const DsiRoomMetadata *room);
bool DsiRoomLinkOriginE39_Ready(void);
unsigned DsiRoomLinkOriginE39_Reason(void);
#endif
