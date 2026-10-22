#ifndef DSI_COLLISION_AUDIT_E35_H
#define DSI_COLLISION_AUDIT_E35_H
#include <stdbool.h>
/* Read-only audit of original GBA MapLayer collision arrays. No movement. */
bool DsiCollisionAuditE35_Run(unsigned width, unsigned height);
unsigned DsiCollisionAuditE35_NonzeroTiles(void);
unsigned DsiCollisionAuditE35_Errors(void);
#endif
