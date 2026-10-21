#ifndef DSI_ENTITY_STATE_E29_H
#define DSI_ENTITY_STATE_E29_H
#include <stdbool.h>
/* Storage and empty-list invariants for ORIGINAL src/entity.c.
 * No player spawning, entity behavior, AI or fabricated gameplay. */
void DsiEntityState_Reset(void);
bool DsiEntityState_EmptyValid(void);
#endif
