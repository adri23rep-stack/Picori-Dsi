/* E24: Native DSi boundary for ORIGINAL GameTask TRANSITION / INIT plus
 * a guarded room-state prefix. Deferred services are NOT game implementations.
 * Never dispatch active gameplay or persistent save handlers.
 */
#include "dsi_game_native.h"
#include "dsi_game_transition.h"
#include "dsi_game_bootstrap.h"
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

RoomTransition gRoomTransition;
RoomControls gRoomControls;
OAMControls gOAMControls;
PauseMenuOptions gPauseMenuOptions;
PlayerEntity gPlayerEntity;
PlayerState gPlayerState;
DiggingCaveEntranceTransition gDiggingCaveEntranceTransition;
u8 gUpdateVisibleTiles;
const OverworldLocation gOverworldLocations[1] = {0}; /* not accessed */
static DsiGameNativeStats sStats;
static bool sAllowRoomTick;
/* The GBA linker allocates gPriorityHandler at a fixed RAM address; DSi
 * needs the actual variable; other priority behaviors remain in src/entity.c. */
PriorityHandler gPriorityHandler;
/* Temporary DSi adapter for src/entity.c's original priority setter.
 * The full entity system is not linkable yet; this is the EXACT one-line
 * original assignment, not a replacement for entity/gameplay handling. */
void SetInitializationPriority(void) {
    gPriorityHandler.event_priority = PRIO_HIGHEST;
}

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

void DsiGameNative_Reset(void) {
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

    sAllowRoomTick = true;
    GameTask(); /* ORIGINAL GameTask_Main -> GameMain_InitRoom prefix */
    sAllowRoomTick = false;
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
