#ifndef TMC_DSI_ENGINE_STATE_H
#define TMC_DSI_ENGINE_STATE_H
#include <stdbool.h>
/* Actual original src/flags.c save/area/room state. Called only before game boot.
 * This is a diagnostic stage, not a substitute for the eventual save backend. */
void DsiEngineState_Reset(void);
bool DsiEngineState_SelfTest(void);
#endif
