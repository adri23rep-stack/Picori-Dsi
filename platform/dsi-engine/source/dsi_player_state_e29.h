#ifndef DSI_PLAYER_STATE_E29_H
#define DSI_PLAYER_STATE_E29_H
#include <stdbool.h>
/* Authentic player interaction storage in DSi RAM, not substitute gameplay. */
void DsiPlayerState_Reset(void);
bool DsiPlayerState_Ready(void);
#endif
