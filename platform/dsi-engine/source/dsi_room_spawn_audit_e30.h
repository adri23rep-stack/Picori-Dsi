#ifndef DSI_ROOM_SPAWN_AUDIT_E30_H
#define DSI_ROOM_SPAWN_AUDIT_E30_H
/* E30.8: read-only inventory of ORIGINAL ROM room entity branch inputs.
 * Reports why native LoadRoomEntity cannot run yet; never fakes the engine. */
#include <stdbool.h>
typedef struct {
    unsigned records;
    unsigned scripted;
    unsigned managers;
    unsigned collision;
    unsigned duplicate_check;
    unsigned unfamiliar_kind;
} DsiRoomSpawnAuditE30;
void DsiRoomSpawnAuditE30_Reset(void);
bool DsiRoomSpawnAuditE30_Run(void);
bool DsiRoomSpawnAuditE30_Ready(void);
const DsiRoomSpawnAuditE30 *DsiRoomSpawnAuditE30_Get(void);
#endif
