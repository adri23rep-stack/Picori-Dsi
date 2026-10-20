#ifndef DSI_GAME_TRANSITION_H
#define DSI_GAME_TRANSITION_H
#include <stdbool.h>
#include "global.h"
/* Carefully bounded native RAM copy destinations for original GameTask.
 * This is a preparation check, NOT the execution of GameTask on DSi. */
bool DsiGameTransition_AllowNativeCopy(const void* src, u32 size);
bool DsiGameTransition_Preflight(void);
#endif
