#ifndef DSI_ROOM_PROPERTY_DATA_E30_H
#define DSI_ROOM_PROPERTY_DATA_E30_H
/* E30.2: native, aligned EntityData materialized byte-for-byte from the
 * genuine EU ROM entity bank snapshots. NOT an alternate entity spawner. */
#include <stdbool.h>
#include <stdint.h>
#include "room.h"
#include "dsi_entity_lists.h"

void DsiRoomPropertyDataE30_Reset(void);
/* Only slots 0/1. Fails closed and keeps last-good copy if inputs are bad. */
bool DsiRoomPropertyDataE30_Bind(void);
/* Ready only if upstream original ROM snapshots STILL match the native copy. */
bool DsiRoomPropertyDataE30_Ready(void);
/* Pure read access; never call LoadRoomEntityList until engine is ported. */
const EntityData *DsiRoomPropertyDataE30_EntityBank(unsigned bank);
unsigned DsiRoomPropertyDataE30_Count(unsigned bank);
#endif
