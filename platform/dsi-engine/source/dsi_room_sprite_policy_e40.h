#ifndef DSI_ROOM_SPRITE_POLICY_E40_H
#define DSI_ROOM_SPRITE_POLICY_E40_H
#include <stdbool.h>
#include "dsi_room_probe.h"
#include "entity.h"
/* Rendering eligibility, separate from the REAL game's entity action tick.
 * The source-correct original Archway() MUST continue to execute even when
 * this experimental graphics compositing path is quarantined. */
typedef enum {
    DSI_E40_ARCH_GFX_INVALID=0,
    DSI_E40_ARCH_GFX_WAITING_FOR_NATIVE_SPRITE_PIPELINE=1
} DsiRoomE40GfxDecision;
DsiRoomE40GfxDecision DsiRoomSpritePolicyE40_Archway(const DsiRoomMetadata *room,const Entity *arch);
bool DsiRoomSpritePolicyE40_AllowPreview(const DsiRoomMetadata *room,const Entity *arch);
#endif
