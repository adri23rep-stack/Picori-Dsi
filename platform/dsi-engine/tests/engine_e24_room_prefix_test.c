/* Host reproduction of native DSi guarded GameTask path. No persistent saves. */
#include "dsi_game_bootstrap.h"
#include "dsi_game_native.h"
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
#include "room.h"
#include "screen.h"
#include "entity.h"
#include "fileselect.h"
#include "save.h"
#include <assert.h>
#include <stdio.h>
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
    SetTask(TASK_FILE_SELECT);
    assert(DsiFileSelectProbe_Prepare());
    assert(DsiGameBootstrap_Begin(0));
    DsiGameNative_Reset();
    assert(DsiGameNative_BootTwoStates());
    const DsiGameNativeStats* stats = DsiGameNative_Status();
    assert(stats->attempted && stats->transition_ticks == 1 && stats->init_ticks == 1);
    assert(stats->deferred_services >= 8 && stats->errors == 0);
    assert(gMain.state == GAMETASK_MAIN && gMain.substate == GAMEMAIN_INITROOM);
    assert(gRoomControls.area == 0x22 && gRoomControls.room == 0x15);
    assert(gSave.initialized == 1 && gSave.global_progress == 1);
    assert(gSave.saved_status.start_pos_x == 0x90 && gSave.saved_status.start_pos_y == 0x38);
    assert(gDsiGameBootstrapStats.game_task_linked);
    /* Re-entry and unsupported MAIN must be blocked rather than fake gameplay. */
    assert(!DsiGameNative_BootTwoStates());
    unsigned count = gRoomTransition.frameCount;
    GameTask();
    assert(gRoomTransition.frameCount == count);
    assert(DsiGameNative_BootRoomPrefix());
    assert(stats->room_prefix_ticks == 1);
    assert(gMain.state == GAMETASK_MAIN && gMain.substate == GAMEMAIN_CHANGEROOM);
    assert(gPriorityHandler.event_priority == PRIO_HIGHEST);
    assert(gScreen.lcd.displayControl ==
           (DISPCNT_BG0_ON | DISPCNT_BG1_ON | DISPCNT_BG2_ON | DISPCNT_OBJ_ON | DISPCNT_OBJ_1D_MAP));
    assert(gRoomTransition.transitioningOut == 0);
    assert(!DsiGameNative_BootRoomPrefix());
    count = gRoomTransition.frameCount;
    GameTask(); /* unsupported CHANGEROOM MUST remain fail-closed */
    assert(gRoomTransition.frameCount == count);
    assert(stats->errors == 0);
    printf("PASS E24 HOST: original GameTask TRANSITION -> INIT -> room PREFIX at %u/%u; writes=0\n",
           gRoomControls.area, gRoomControls.room);
    DsiGbaBus_DetachRom();
    TmcRom_Close(&reader);
    return 0;
}
