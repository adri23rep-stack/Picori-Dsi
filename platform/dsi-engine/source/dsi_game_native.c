/* E24: Native DSi boundary for ORIGINAL GameTask TRANSITION / INIT plus
 * a guarded room-state prefix. Deferred services are NOT game implementations.
 * Never dispatch active gameplay or persistent save handlers.
 */
#include "dsi_game_native.h"
#include "dsi_game_transition.h"
#include "dsi_game_bootstrap.h"
#ifdef TMC_DSI_E29_ENTITY_CORE
#include "dsi_entity_state_e29.h"
#include "dsi_collision_state_e29.h"
#include "dsi_player_state_e29.h"
#include "dsi_e30_gate.h"
#include "dsi_player_preflight.h"
#include "dsi_room_properties_e30.h"
#include "dsi_room_property_data_e30.h"
#include "dsi_room_property_native_e30.h"
#endif
#ifdef TMC_DSI_E28_ORIGINAL_INITROOM
#include "dsi_area_headers.h"
#include "dsi_room_resources.h"
#include "area.h"
#include "flags.h"
#endif
#include "main.h"
#include "game.h"
#include "room.h"
#include "screen.h"
#include "fade.h"
#include "save.h"
#include "player.h"
#include "entity.h"
#include "manager/diggingCaveEntranceManager.h"
#include <string.h>
#ifdef TMC_DSI_E26_ORIGINAL_MESSAGE
#include "message.h"
#include "structures.h"
/* Original engine variables relocated to native DSi memory. Window
 * globals are already defined inside original src/message.c (PC_PORT). */
Message gMessage;
TextRender gTextRender;
struct_02000040 gUnk_02000040;
#endif

RoomTransition gRoomTransition;
RoomControls gRoomControls;
OAMControls gOAMControls;
PauseMenuOptions gPauseMenuOptions;
PlayerEntity gPlayerEntity;
PlayerState gPlayerState;
DiggingCaveEntranceTransition gDiggingCaveEntranceTransition;
u8 gUpdateVisibleTiles;
/* E27: gOverworldLocations is linked directly from ORIGINAL src/data/areaMetadata.c. */
static DsiGameNativeStats sStats;
static bool sAllowRoomTick;
/* The GBA linker allocates gPriorityHandler at a fixed RAM address; DSi
 * needs the actual variable; other priority behaviors remain in src/entity.c. */
PriorityHandler gPriorityHandler;
/* Temporary DSi adapter for src/entity.c's original priority setter.
 * The full entity system is not linkable yet; this is the EXACT one-line
 * original assignment, not a replacement for entity/gameplay handling. */
#ifndef TMC_DSI_E29_ENTITY_CORE
void SetInitializationPriority(void) {
    gPriorityHandler.event_priority = PRIO_HIGHEST;
}
#endif
/* E29.4 uses the actual SetInitializationPriority from src/entity.c.
 * This fallback only exists for tests and older build configurations. */

void ClearRoomMemory(void) { ++sStats.deferred_services; }
void ClearArmosData(void) { ++sStats.deferred_services; }
void ClearMenuSavestate(void) { ++sStats.deferred_services; }
void ResetUI(void) { ++sStats.deferred_services; }
void ResetTmpFlags(void) { ++sStats.deferred_services; }
void __wrap_zMallocInit(void) { ++sStats.deferred_services; }
void SetBGDefaults(void) { ++sStats.deferred_services; }
void ClearTileMaps(void) { ++sStats.deferred_services; }
void sub_0806FD8C(void) { ++sStats.deferred_services; }
void LoadGfxGroups(void) { ++sStats.deferred_services; }

#ifdef TMC_DSI_E28_ORIGINAL_INITROOM
/* Link-time tap: verifies the REAL InitRoom in src/gameUtils.c ran, not a
 * dummy status updated by the diagnostic renderer. */
void __real_InitRoom(void);
void __wrap_InitRoom(void) {
    ++sStats.original_room_calls;
    __real_InitRoom();
}
#endif

/* PC_PORT MemFill32 treats all 0x02xxxxxx pointers as GBA shadow RAM.
 * On DSi the real ARM9 player state also occupies this range. Override
 * ONLY the genuine path_memory write; preserve prior behavior elsewhere. */
void __real_MemFill32(u32 value, void* dest, u32 size);
void __wrap_MemFill32(u32 value, void* dest, u32 size) {
    if (dest == (void*)&gPlayerState.path_memory &&
        size == sizeof(gPlayerState.path_memory)) {
        if (size & 3u) { ++sStats.errors; return; }
        u32 *p = (u32*)dest;
        for (u32 i = 0; i < size / sizeof(u32); ++i) p[i] = value;
        return;
    }
    __real_MemFill32(value, dest, size);
}

void DsiGameNative_Reset(void) {
#ifdef TMC_DSI_E29_ENTITY_CORE
    /* Allocate/reset ORIGINAL game entity pool and 9 self-sentinel lists.
     * Does not spawn Link or advance any entity tick. */
    DsiEntityState_Reset();
    DsiCollisionState_Reset();
    /* E29.6: the unmodified playerUtils.c interaction reset. */
    DsiPlayerState_Reset();
    DsiE30Gate_Reset();
    DsiRoomPropertiesE30_Reset();
    /* Detach before invalidating property source banks. */
    DsiRoomPropertyNativeE30_Reset();
    DsiRoomPropertyDataE30_Reset();
#endif
    memset(&sStats, 0, sizeof(sStats));
    sAllowRoomTick = false;
    memset(&gPriorityHandler, 0, sizeof(gPriorityHandler));
    memset(&gRoomTransition, 0, sizeof(gRoomTransition));
    memset(&gRoomControls, 0, sizeof(gRoomControls));
    memset(&gOAMControls, 0, sizeof(gOAMControls));
}

const DsiGameNativeStats *DsiGameNative_Status(void) { return &sStats; }

bool DsiGameNative_BootTwoStates(void) {
    if (!DsiGameTransition_Preflight() || sStats.attempted) return false;
    sStats.attempted = true;
    /* No game fade scheduler exists yet. This is an explicitly requested
     * new-game diagnostic with no file I/O and no room state execution. */
    gFadeControl.active = 0;
    GameTask(); /* ORIGINAL GameTask_Transition, FinalizeSave */
    if (gMain.task != TASK_GAME || gMain.state != GAMETASK_INIT ||
        gMain.substate != GAMEMAIN_INITROOM ||
        gSave.saved_status.area_next != 0x22 ||
        gSave.saved_status.room_next != 0x15 ||
        gRoomTransition.player_status.area_next != 0x22 ||
        gRoomTransition.player_status.room_next != 0x15) {
        ++sStats.errors;
        return false;
    }
    ++sStats.transition_ticks;
    GameTask(); /* ORIGINAL GameTask_Init; guarded services */
    if (gMain.task != TASK_GAME || gMain.state != GAMETASK_MAIN ||
        gMain.substate != GAMEMAIN_INITROOM ||
        gRoomControls.area != 0x22 || gRoomControls.room != 0x15) {
        ++sStats.errors;
        return false;
    }
    ++sStats.init_ticks;
    gDsiGameBootstrapStats.game_task_linked = true;
    return true;
}

bool DsiGameNative_AllowRoomTick(void) { return sAllowRoomTick; }

bool DsiGameNative_BootRoomPrefix(void) {
    if (!sStats.attempted || sStats.transition_ticks != 1 ||
        sStats.init_ticks != 1 || sStats.room_prefix_ticks || sStats.errors ||
        gMain.task != TASK_GAME || gMain.state != GAMETASK_MAIN ||
        gMain.substate != GAMEMAIN_INITROOM ||
        gRoomControls.area != 0x22 || gRoomControls.room != 0x15)
        return false;

#ifdef TMC_DSI_E28_ORIGINAL_INITROOM
    /* Fail closed if either immutable EU ROM snapshot is absent. The older
     * code silently skipped original InitRoom yet advanced to CHANGEROOM. */
    const DsiRoomResources *res = DsiRoomResources_Current();
    const RoomHeader *hdr = DsiAreaHeaders_Get(gRoomControls.area, gRoomControls.room);
    if (!res || !hdr || !res->valid ||
        res->area != gRoomControls.area || res->room != gRoomControls.room ||
        hdr->map_x != res->map_x || hdr->map_y != res->map_y ||
        hdr->pixel_width != res->width || hdr->pixel_height != res->height ||
        hdr->tileSet_id != res->tileSet_id) {
        ++sStats.errors;
        return false;
    }
#endif

    sAllowRoomTick = true;
    GameTask(); /* ORIGINAL GameTask_Main -> GameMain_InitRoom prefix */
    sAllowRoomTick = false;
#ifdef TMC_DSI_E28_ORIGINAL_INITROOM
    /* The room was initialized by ORIGINAL engine functions. Prove both the
     * invocation and the original structure fields, not merely task state. */
    const RoomResInfo *info = gArea.pCurrentRoomInfo;
    if (sStats.original_room_calls != 1u ||
        info != &gArea.roomResInfos[gRoomControls.room] ||
        gArea.flag_bank != gAreaMetadata[gRoomControls.area].flag_bank ||
        gArea.localFlagOffset != gLocalFlagBanks[gArea.flag_bank] ||
        gArea.portal_timer != 180u || gArea.lightLevel != 256u ||
        info->map_x != res->map_x || info->map_y != res->map_y ||
        info->pixel_width != res->width || info->pixel_height != res->height ||
        !info->map || !info->tileSet || !info->tiles) {
        ++sStats.errors;
        return false;
    }
#endif
    if (gMain.task != TASK_GAME || gMain.state != GAMETASK_MAIN ||
        gMain.substate != GAMEMAIN_CHANGEROOM ||
        gRoomTransition.transitioningOut != 0 ||
        gRoomTransition.field_0x4[0] != 0 ||
        gRoomTransition.field_0x4[1] != 0 ||
        gScreen.lcd.displayControl !=
            (DISPCNT_BG0_ON | DISPCNT_BG1_ON | DISPCNT_BG2_ON |
             DISPCNT_OBJ_ON | DISPCNT_OBJ_1D_MAP) ||
        gPriorityHandler.event_priority != PRIO_HIGHEST) {
        ++sStats.errors;
        return false;
    }
    ++sStats.room_prefix_ticks;
    return true;
}

/* E29.8: REAL gameUtils.c::InitializePlayer with original list, collision
 * and the existing 3DS source translation of the Thumb sprite layer.
 * Separate from InitializeEntities (still blocked on room properties).
 * May run only ONCE after original InitRoom completed and ROM EU preflight.
 */
bool DsiGameNative_BootOriginalPlayer(void) {
    if (!sStats.attempted || sStats.errors || sStats.room_prefix_ticks != 1u ||
        sStats.original_room_calls != 1u || sStats.original_player_calls ||
        gMain.task != TASK_GAME || gMain.state != GAMETASK_MAIN ||
        gMain.substate != GAMEMAIN_CHANGEROOM ||
        !DsiPlayerPreflight_Ready() ||
        !DsiEntityState_EmptyValid() ||
        !DsiCollisionState_EmptyValid() ||
        !DsiPlayerState_Ready()) return false;

    InitializePlayer(); /* AUTHENTIC src/gameUtils.c, not a game shim. */
    LinkedList *list = &gEntityLists[1];
    Entity *pl = &gPlayerEntity.base;
    if (pl->kind != PLAYER || pl->health != gSave.stats.health ||
        pl->x.HALF.HI != gRoomTransition.player_status.start_pos_x ||
        pl->y.HALF.HI != gRoomTransition.player_status.start_pos_y ||
        gRoomControls.camera_target != pl ||
        list->first != pl || list->last != pl ||
        pl->prev != (Entity*)(void*)list ||
        pl->next != (Entity*)(void*)list ||
        gEntCount != 1 || gManagerCount != 0 ||
        !DsiCollisionState_PlayerRegistered() ||
        !DsiPlayerState_Ready()) {
        ++sStats.errors;
        return false;
    }
    /* E30.6: ORIGINAL src/entity.c::DeepFindEntityByID is now linked
     * and exercised on ARM9's real intrusive list containing Link.
     * Two read-only lookups: one hit, one guaranteed missing ID.
     * This never allocates NPCs or runs LoadRoomEntity. */
    if (DeepFindEntityByID(PLAYER, pl->id) != pl ||
        DeepFindEntityByID(PLAYER, (u8)(pl->id ^ 0x80u)) != NULL ||
        gEntCount != 1u || gManagerCount != 0u ||
        list->first != pl || list->last != pl) {
        ++sStats.errors;
        return false;
    }
    sStats.original_entity_lookups = 2u;
    ++sStats.original_player_calls;
    return true;
}
bool DsiGameNative_OriginalEntityLookupReady(void) {
    return sStats.original_player_calls == 1u &&
           sStats.original_entity_lookups == 2u &&
           sStats.errors == 0u;
}
