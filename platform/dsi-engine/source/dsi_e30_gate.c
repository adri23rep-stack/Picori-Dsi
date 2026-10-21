/* E30 safety preparation. READS original engine globals, never simulates
 * input, game time, Link movement, sprite rendering, or hitbox collisions.
 * Missing native sprite-layer integration is explicitly BLOCKING.
 */
#include "dsi_e30_gate.h"
#include "dsi_game_native.h"
#include "dsi_player_state_e29.h"
#include "dsi_collision_state_e29.h"
#include "dsi_room_properties_e30.h"
#include "main.h"
#include "game.h"
#include "room.h"
#include "area.h"
#include "entity.h"
#include "player.h"

static bool sOriginalEntitiesComplete;
void DsiE30Gate_Reset(void) { sOriginalEntitiesComplete = false; }
void DsiE30Gate_OriginalEntitiesCompleted(void) { sOriginalEntitiesComplete = true; }
unsigned DsiE30Gate_Blockers(void) {
    unsigned blocked = 0u;
    const DsiGameNativeStats *st = DsiGameNative_Status();
    Entity *link = &gPlayerEntity.base;
    LinkedList *list = &gEntityLists[1];
    const DsiRoomPropertiesE30 *source = DsiRoomPropertiesE30_Current();
    if (!st || st->room_prefix_ticks != 1u || st->original_room_calls != 1u ||
        st->errors || gRoomControls.area != 0x22u || gRoomControls.room != 0x15u)
        blocked |= DSI_E30_BLOCK_ROOM;
    if (gMain.task != TASK_GAME || gMain.state != GAMETASK_MAIN ||
        (gMain.substate != GAMEMAIN_CHANGEROOM &&
         gMain.substate != GAMEMAIN_UPDATE))
        blocked |= DSI_E30_BLOCK_STATE;
    if (gRoomControls.camera_target != link || link->kind != PLAYER ||
        (link->flags & (ENT_COLLIDE | ENT_PERSIST)) != (ENT_COLLIDE | ENT_PERSIST) ||
        link->health == 0u)
        blocked |= DSI_E30_BLOCK_PLAYER;
    if (gEntCount == 0 || list->first != link || list->last != link ||
        link->prev != (Entity*)(void*)list ||
        link->next != (Entity*)(void*)list)
        blocked |= DSI_E30_BLOCK_LIST;
    if (!DsiCollisionState_PlayerRegistered())
        blocked |= DSI_E30_BLOCK_HITBOX;
    if (!DsiPlayerState_Ready())
        blocked |= DSI_E30_BLOCK_INTERACTION;
    /* E30.1 snapshot proves ROM bytes were copied, NOT that the engine may
     * invoke GBA callbacks. Require both the actual native room wiring and
     * a later explicit validation of dispatch, not a fabricated pointer. */
    if (!source || !source->valid ||
        source->area != gRoomControls.area ||
        source->room != gRoomControls.room ||
        !source->native_dispatch_ready ||
        !gArea.pCurrentRoomInfo || !gArea.pCurrentRoomInfo->properties)
        blocked |= DSI_E30_BLOCK_PROPERTIES;
    if (!sOriginalEntitiesComplete)
        blocked |= DSI_E30_BLOCK_ENTITIES_INIT;
#ifndef TMC_DSI_E30_NATIVE_SPRITE_LAYER_VERIFIED
    /* Explicitly do not claim this function works on ARM9 until the
     * GBA sprite-layer behavior has a verified native DSi equivalent. */
    blocked |= DSI_E30_BLOCK_SPRITE_LAYER;
#endif
    return blocked;
}
bool DsiE30Gate_Ready(void) { return DsiE30Gate_Blockers() == 0u; }
