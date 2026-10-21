#ifndef DSI_ROOM_REGISTRY_PROBE_E30_H
#define DSI_ROOM_REGISTRY_PROBE_E30_H
#include <stdbool.h>
/* E30.7: executable proof of the unmodified original src/room.c
 * RegisterRoomEntity + src/entity.c AppendEntityToList/DeepFindEntityByID.
 * Uses a single empty native pool slot and immediately restores it.
 * NEVER persists a fabricated NPC or calls LoadRoomEntity / GameTask. */
void DsiRoomRegistryProbeE30_Reset(void);
bool DsiRoomRegistryProbeE30_Run(void);
bool DsiRoomRegistryProbeE30_Passed(void);
#endif
