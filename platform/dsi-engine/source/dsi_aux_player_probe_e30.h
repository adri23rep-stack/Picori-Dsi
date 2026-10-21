#ifndef DSI_AUX_PLAYER_PROBE_E30_H
#define DSI_AUX_PLAYER_PROBE_E30_H
#include <stdbool.h>
/* Probe unmodified original src/entity.c CreateAuxPlayerEntity on genuine
 * DS RAM. No auxiliary player registered, added, or updated. */
void DsiAuxPlayerProbeE30_Reset(void);
bool DsiAuxPlayerProbeE30_Run(void);
bool DsiAuxPlayerProbeE30_Passed(void);
#endif
