/* HOST TEST SHIMS: not used by the DSi executable. Explicit counters make
 * intercepted services visible, and never write persistent save data. */
#include "main.h"
#include "game.h"
#include "room.h"
#include "screen.h"
#include "fade.h"
#include "player.h"
#include "message.h"
#include "structures.h"
#include "manager/diggingCaveEntranceManager.h"
#include "save.h"
#include <stdlib.h>
RoomTransition gRoomTransition;
RoomControls gRoomControls;
OAMControls gOAMControls;
Message gMessage;
PriorityHandler gPriorityHandler;
PauseMenuOptions gPauseMenuOptions;
PlayerEntity gPlayerEntity;
PlayerState gPlayerState;
DiggingCaveEntranceTransition gDiggingCaveEntranceTransition;
u8 gUpdateVisibleTiles;
const OverworldLocation gOverworldLocations[1] = {0}; /* never dereferenced */
unsigned e19_finalize_would_save, e19_room_clears, e19_armos_clears,
    e19_menu_clears, e19_ui_resets, e19_tmp_flags_resets, e19_heap_resets;
/* ORIGINAL FinalizeSave() from src/playerUtils.c now linked, RAM-only. */
void ClearRoomMemory(void) { ++e19_room_clears; }
void ClearArmosData(void) { ++e19_armos_clears; }
void ClearMenuSavestate(void) { ++e19_menu_clears; }
void ResetUI(void) { ++e19_ui_resets; }
void ResetTmpFlags(void) { ++e19_tmp_flags_resets; }
void __wrap_zMallocInit(void) { ++e19_heap_resets; }

/* First INIT-only adapters. They are counters, NOT graphics/room systems. */
unsigned e19_bg_defaults, e19_tilemap_clears, e19_collision_resets,
         e19_room_gfx_requests;
void SetBGDefaults(void) { ++e19_bg_defaults; }
void ClearTileMaps(void) { ++e19_tilemap_clears; }
void sub_0806FD8C(void) { ++e19_collision_resets; }
void LoadGfxGroups(void) { ++e19_room_gfx_requests; }
