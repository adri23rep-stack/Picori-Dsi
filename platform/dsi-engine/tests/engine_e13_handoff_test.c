/* E13: genuine original title button handling and task handoff; no fake file menu. */
#include "dsi_title_runner.h"
#include "dsi_title_trace.h"
#include "dsi_title_assets.h"
#include "dsi_title_entities.h"
#include "dsi_title_sprites.h"
#include "dsi_title_object_gfx.h"
#include "dsi_gba_bus.h"
#include "tmc_rom_stream.h"
#include "port_gba_mem.h"
#include "main.h"
#include "common.h"
#include "game.h"
#include <assert.h>
#include <stdio.h>
#include <string.h>
Input gInput;
Main gMain;
extern u8 _gMenuSharedStorage[0x40];
static void keys(uint16_t pressed) {
    /* Real REG_KEYINPUT transport, then UNMODIFIED ReadKeyInput(). */
    gba_write16(0x04000130u,(uint16_t)(~pressed)&0x03ffu);
    ReadKeyInput();
}
int main(int argc, char**argv) {
    assert(argc>=2);
    TmcRomReader rom={0};
    DsiGbaBus_Init();
    assert(TmcRom_Open(&rom,argv[1]));
    assert(rom.region==TMC_ROM_REGION_EUROPE);
    DsiGbaBus_AttachRom(&rom);
    DsiTitleAssets_Enable(true,1);
    DsiTitleSprites_Enable(true);
    DsiTitleEntities_Enable(true);
    DsiTitleAssets_ResetStats();
    assert(DsiTitleTrace_TestFirstTransitions());
    DsiTitleRunner runner;
    DsiTitleRunner_Init(&runner);
    for(int i=0;i<850;i++) {
        keys(0);
        assert(DsiTitleRunner_Step(&runner)==DSI_TITLE_RUNNING);
    }
    assert(gMain.task==TASK_TITLE);
    assert(_gMenuSharedStorage[5]==3); /* ORIGINAL EU title interactive state */
    assert(gUnk_02000010.listenForKeyPresses);
    assert(gDsiTitleEntityStats.created==4);
    keys(0x0008); /* START, the GBA's real button bit */
    assert(gInput.newKeys==0x0008);
    assert(DsiTitleRunner_Step(&runner)==DSI_TITLE_RUNNING);
    assert(gMain.task==TASK_TITLE && gUI.lastState==2);
    keys(0);
    assert(DsiTitleRunner_Step(&runner)==DSI_TITLE_RUNNING);
    assert(DsiTitleRunner_Step(&runner)==DSI_TITLE_AT_FILE_SELECT);
    assert(gMain.task==TASK_FILE_SELECT);
    assert(runner.entered_file_select_at==runner.title_ticks);
    unsigned previous=runner.title_ticks;
    /* This extra call must NOT run TitleTask again on FILE_SELECT state. */
    for(int n=0;n<5;n++)
        assert(DsiTitleRunner_Step(&runner)==DSI_TITLE_AT_FILE_SELECT);
    assert(runner.title_ticks==previous);
    assert(gDsiTitleAssetStats.errors==0);
    assert(gDsiTitleObjectGfxStats.errors==0);
    assert(gDsiTitleEntityStats.errors==0);
    assert(gDsiTitleSpriteStats.errors==0);
    assert(DsiGbaBus_InvalidReads()==0 && DsiGbaBus_InvalidWrites()==0);
    printf("PASS E13: Original START -> task %u (file select) after %u title ticks; safe stop, 0 asset errors\n",gMain.task,runner.title_ticks);
    /* Reboot the real title task, repeat with A instead of START. */
    memset(&gInput, 0, sizeof(gInput));
    DsiTitleAssets_ResetStats();
    assert(DsiTitleTrace_TestFirstTransitions());
    DsiTitleRunner_Init(&runner);
    for (int i=0; i<850; ++i) {
        keys(0);
        assert(DsiTitleRunner_Step(&runner)==DSI_TITLE_RUNNING);
    }
    assert(_gMenuSharedStorage[5]==3);
    keys(1); /* original A_BUTTON */
    assert(gInput.newKeys==1);
    assert(DsiTitleRunner_Step(&runner)==DSI_TITLE_RUNNING);
    keys(0);
    assert(DsiTitleRunner_Step(&runner)==DSI_TITLE_RUNNING);
    assert(DsiTitleRunner_Step(&runner)==DSI_TITLE_AT_FILE_SELECT);
    assert(gMain.task==TASK_FILE_SELECT);
    printf("PASS E13: Original A -> file select after %u title ticks\n",runner.title_ticks);
    /* Rejected tasks and budget are explicit; no unexpected code is run. */
    DsiTitleRunner_Init(&runner);
    gMain.task=TASK_GAME;
    assert(DsiTitleRunner_Step(&runner)==DSI_TITLE_UNEXPECTED_TASK);
    assert(runner.title_ticks==0);
    DsiTitleRunner_Init(&runner);
    gMain.task=TASK_TITLE;
    runner.title_ticks=1500;
    assert(DsiTitleRunner_Step(&runner)==DSI_TITLE_TICK_LIMIT);
    assert(runner.title_ticks==1500);
    puts("PASS E13: unported task and title-frame budget are safely rejected");
    DsiGbaBus_DetachRom();
    TmcRom_Close(&rom);
    return 0;
}
