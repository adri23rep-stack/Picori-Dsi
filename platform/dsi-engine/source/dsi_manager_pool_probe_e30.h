#ifndef DSI_MANAGER_POOL_PROBE_E30_H
#define DSI_MANAGER_POOL_PROBE_E30_H
#include <stdbool.h>
/* E30.9: original src/entity.c::GetEmptyManager read-only on ARM9.
 * Never initializes or inserts a new Manager, never spawns anything. */
void DsiManagerPoolProbeE30_Reset(void);
bool DsiManagerPoolProbeE30_Run(void);
bool DsiManagerPoolProbeE30_Passed(void);
#endif
