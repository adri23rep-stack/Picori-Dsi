#ifndef DSI_COLLISION_PROBE_E42_H
#define DSI_COLLISION_PROBE_E42_H
#include <stdbool.h>
#include "entity.h"
extern u8 gDsiE42CollisionParams[7][96];
bool DsiCollisionE42_Prepare(void);
void DsiCollisionE42_Reset(void);
bool DsiCollisionE42_Ready(void);
bool DsiCollisionE42_Point(const Entity* entity,u32 roomX,u32 roomY,u32* blocked);
bool DsiCollisionE42_Probe(Entity* entity);
const u16* DsiCollisionE42_Shape(u32 index);
void* DsiCollisionE42_Hitbox(uintptr_t pointer);
u32 DsiCollisionE42_OriginalPoint(Entity* entity,u32 x,u32 y);
#endif
