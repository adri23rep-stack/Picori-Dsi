/* E19 HOST-ONLY: Execute ORIGINAL GameTask() first state with guarded
 * environmental shims. This is NOT a functional room or native DSi loop. */
#include "dsi_game_bootstrap.h"
#include "dsi_game_transition.h"
#include "dsi_file_select_probe.h"
#include "dsi_title_trace.h"
#include "dsi_gba_bus.h"
#include "dsi_title_assets.h"
#include "dsi_title_sprites.h"
#include "dsi_title_entities.h"
#include "tmc_rom_stream.h"
#include "main.h"
#include "common.h"
#include "game.h"
#include "fade.h"
#include "room.h"
#include "fileselect.h"
#include "save.h"
#include <assert.h>
#include <stdio.h>
#include <string.h>
extern void GameTask(void); /* UNMODIFIED src/game.c */
extern unsigned e19_finalize_would_save, e19_room_clears, e19_armos_clears,
    e19_menu_clears, e19_ui_resets, e19_tmp_flags_resets, e19_heap_resets;
extern unsigned e19_bg_defaults, e19_tilemap_clears, e19_collision_resets, e19_room_gfx_requests;
Input gInput;
Main gMain;
int main(int argc, char **argv) {
    assert(argc >= 2);
    TmcRomReader reader = {0};
    DsiGbaBus_Init();
    assert(TmcRom_Open(&reader, argv[1]));
    assert(reader.region == TMC_ROM_REGION_EUROPE);
    DsiGbaBus_AttachRom(&reader);
    DsiTitleAssets_Enable(true, RegionDefaultLanguage());
    DsiTitleSprites_Enable(true);
    DsiTitleEntities_Enable(true);
    assert(DsiTitleTrace_TestFirstTransitions());
    /* Bypass rendering for test only; original file selection data routines. */
    SetTask(TASK_FILE_SELECT);
    assert(DsiFileSelectProbe_Prepare());
    assert(DsiGameBootstrap_Begin(0));
    assert(gMain.task == TASK_GAME && gMain.state == GAMETASK_TRANSITION);
    assert(DsiGameTransition_Preflight());
    assert(!DsiGameTransition_AllowNativeCopy((const void*)0x08001000u, 4));
    /* Observe fade wait in original GameTask: no services may run yet. */
    gRoomTransition.frameCount = 40;
    gFadeControl.active = 1;
    GameTask();
    assert(gMain.state == GAMETASK_TRANSITION);
    assert(gRoomTransition.frameCount == 41);
    assert(gSave.initialized == 0);
    /* The real transition may now run; side-effect-heavy services are
     * intercepted exclusively inside this host harness. */
    gFadeControl.active = 0;
    GameTask();
    assert(gMain.task == TASK_GAME);
    assert(gMain.state == GAMETASK_INIT);
    assert(gMain.substate == GAMEMAIN_INITROOM);
    assert(gRoomTransition.player_status.area_next == gSave.saved_status.area_next);
    assert(gRoomTransition.player_status.room_next == gSave.saved_status.room_next);
    assert(memcmp(&gRoomTransition.player_status, &gSave.saved_status,
                  sizeof(gRoomTransition.player_status)) == 0);
    assert(gRoomTransition.type == TRANSITION_FADE_BLACK_SLOW);
    assert(gSave.initialized == 1 && gSave.global_progress == 1);
    assert(gSave.saved_status.area_next == 0x22);
    assert(gSave.saved_status.room_next == 0x15);
    assert(gSave.saved_status.start_pos_x == 0x90);
    assert(gSave.saved_status.start_pos_y == 0x38);
    assert(memcmp(gSave.name, "LIN", 3) == 0);
    assert(e19_room_clears == 1 && e19_armos_clears == 1);
    assert(e19_menu_clears == 1 && e19_ui_resets == 1);
    assert(e19_tmp_flags_resets == 1 && e19_heap_resets == 1);
    /* Execute ORIGINAL GameTask_Init: it maps saved area/room into room
     * controls and advances to GAMETASK_MAIN. Calls to missing subsystems
     * are counted, not faked to load graphics or an actual room. */
    GameTask();
    assert(gMain.state == GAMETASK_MAIN);
    assert(gMain.substate == GAMEMAIN_INITROOM);
    assert(gRoomControls.area == 0x22 && gRoomControls.room == 0x15);
    assert(gFadeControl.mask == 0xffffffffu);
    assert(e19_bg_defaults == 1 && e19_tilemap_clears == 1);
    assert(e19_collision_resets == 1 && e19_room_gfx_requests == 1);
    assert(gDsiGameBootstrapStats.ram_only && !gDsiGameBootstrapStats.game_task_linked);
    printf("PASS E19 HOST: unchanged GameTask() waited for fade, ran TRANSITION+INIT then entered GAMETASK_MAIN; room=%u/%u, disk writes=0 (original FinalizeSave)\n",
           (unsigned)gRoomTransition.player_status.area_next,
           (unsigned)gRoomTransition.player_status.room_next);
    puts("LIMIT: 10+ service calls are HOST TEST SHIMS; GameMain_InitRoom/room rendering NOT run");
    DsiGbaBus_DetachRom();
    TmcRom_Close(&reader);
    return 0;
}
