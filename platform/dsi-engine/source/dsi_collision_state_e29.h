#ifndef DSI_COLLISION_STATE_E29_H
#define DSI_COLLISION_STATE_E29_H
#include <stdbool.h>
/* Genuine GBA collision-list globals moved to DSi ARM9 memory.
 * No collision algorithm or gameplay replacement is implemented. */
void DsiCollisionState_Reset(void);
bool DsiCollisionState_EmptyValid(void);
bool DsiCollisionState_PlayerRegistered(void);
#endif
