/* E14: genuine original title button handling and task handoff; no fake file menu. */
#include "dsi_title_runner.h"
#include "dsi_file_select_probe.h"
#include "dsi_file_select_palette.h"
#include "dsi_title_trace.h"
#include "dsi_title_assets.h"
#include "dsi_title_entities.h"
#include "dsi_title_sprites.h"
#include "dsi_title_object_gfx.h"
#include "dsi_gba_bus.h"
#include "dsi_rom_refs.h"
#include "tmc_rom_stream.h"
#include "port_gba_mem.h"
#include "main.h"
#include "common.h"
#include "game.h"
#include "fileselect.h"
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
    DsiTitleAssets_Enable(true,RegionDefaultLanguage());
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
    /* Original fileselect.c: prepare three in-RAM default files only.
     * Disk saved games are not read or written by the E14 probe. */
    assert(DsiFileSelectProbe_Prepare());
    assert(gDsiFileSelectProbeStats.prepared_slots == 3);
    assert(DsiFileSelectProbe_TestOptions());
    assert(gDsiFileSelectProbeStats.options_tested == 4);
    assert(DsiFileSelectProbe_LoadIntroAssets());
    assert(gDsiFileSelectProbeStats.loaded_gfx_bytes > 0);
    assert(gDsiFileSelectProbeStats.loaded_pal_bytes > 0);
    assert(gDsiFileSelectProbeStats.intro_loads == 1);
    assert(gFileSelectState.unk3 == 7);
    assert(DsiFileSelectProbe_LoadLanguageAssets());
    assert(gDsiFileSelectProbeStats.language_loads == 1);
    assert(gDsiFileSelectProbeStats.language_gfx_bytes > 0);
    printf("PASS E16: file-select language groups 6/15 loaded %u bytes from EU ROM\n",
           gDsiFileSelectProbeStats.language_gfx_bytes);
    /* E17: ROM-backed data window + ORIGINAL menu palette cycling. */
    assert(!DsiFileSelectPalette_Ready());
    assert(DsiFileSelectPalette_Initialize());
    assert(DsiFileSelectPalette_Ready());
    gFileSelectState.unk1 = 1;
    gFileSelectState.unk2 = 1;  /* original increments to index 2, offset 0x20 */
    assert(DsiFileSelectPalette_Step());
    assert(gDsiFileSelectPaletteTicks == 1);
    assert(gFileSelectState.unk2 == 2);
    assert(gFileSelectState.unk1 == 16);
    for (unsigned bank=11; bank<=14; ++bank) {
        const unsigned offsets[4]={0x11A60,0x11B60,0x11C60,0x11D60};
        u8 expected[32];
        assert(DsiGbaBus_Read(0x085A23D0u + 0x20u + offsets[bank-11], expected, 32));
        assert(memcmp(&gPaletteBuffer[bank*16],expected,32)==0);
    }
    assert((gUsedPalettes & 0x7800u)==0x7800u);
    assert(DsiFileSelectPalette_Step());
    assert(gDsiFileSelectPaletteTicks == 2);
    assert(gFileSelectState.unk2 == 2);
    /* Run the full sixteen-frame hold. Next original palette step uses
     * offset 0x40, not the previous cached palette 0x20. */
    for (int frame=0; frame<15; ++frame) assert(DsiFileSelectPalette_Step());
    assert(gDsiFileSelectPaletteTicks == 17);
    assert(gFileSelectState.unk2 == 3);
    for (unsigned bank=11; bank<=14; ++bank) {
        const unsigned offsets[4]={0x11A60,0x11B60,0x11C60,0x11D60};
        u8 expected[32];
        assert(DsiGbaBus_Read(0x085A23D0u + 0x40u + offsets[bank-11], expected, 32));
        assert(memcmp(&gPaletteBuffer[bank*16],expected,32)==0);
    }
    printf("PASS E17: original file-select palette cycle, 4 EU banks byte-exact over %u frames\n",
           gDsiFileSelectPaletteTicks);
    /* A repeated language check should not load the same groups twice. */
    assert(DsiFileSelectProbe_LoadLanguageAssets());
    assert(gDsiFileSelectProbeStats.language_loads == 1);

    /* Independently decode the first EU graphics and palette records.
     * Compare the shadow VRAM/palette to raw ROM, NOT to our own loader. */
    {
        uint32_t group_ptr = 0, command = 0, dest = 0, pal_ptr = 0, pal_record = 0;
        uint8_t actual[64], original[64];
        assert(DsiRomRefs_ReadU32(0x08100204u + 5u * 4u, &group_ptr));
        assert(DsiRomRefs_ReadU32(group_ptr, &command));
        assert(DsiRomRefs_ReadU32(group_ptr + 4u, &dest));
        assert(dest == 0x06004000u);
        assert(DsiGbaBus_Read(dest, actual, sizeof(actual)));
        assert(DsiGbaBus_Read(0x085A23D0u + (command & 0xFFFFFFu), original, sizeof(original)));
        assert(memcmp(actual, original, sizeof(actual)) == 0);
        assert(DsiRomRefs_ReadU32(0x080FED88u + 9u * 4u, &pal_ptr));
        assert(DsiRomRefs_ReadU32(pal_ptr, &pal_record));
        assert(((pal_record >> 16) & 255u) == 0);
        assert(DsiGbaBus_Read(0x05000000u, actual, sizeof(actual)));
        assert(DsiGbaBus_Read(0x085A23D0u + (pal_record & 65535u) * 32u,
                              original, sizeof(original)));
        assert(memcmp(actual, original, sizeof(actual)) == 0);
        printf("PASS E16: original file-select graphics group 5 and palette 9 match EU ROM byte-for-byte\n");
    }
    assert(gDsiFileSelectProbeStats.errors == 0);
    /* Reentering file select should clear the previous session's counters
     * and allow the same unmodified original helpers to load again. */
    assert(DsiFileSelectProbe_Prepare());
    assert(gDsiFileSelectProbeStats.intro_loads == 0);
    assert(gDsiFileSelectProbeStats.language_loads == 0);
    assert(DsiFileSelectProbe_TestOptions());
    assert(DsiFileSelectProbe_LoadIntroAssets());
    assert(DsiFileSelectProbe_LoadLanguageAssets());
    assert(gDsiFileSelectProbeStats.language_loads == 1);
    assert(gDsiFileSelectProbeStats.errors == 0);
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
    printf("PASS E16: Original START -> file-select task after %u title ticks; three original RAM-only save defaults PASS\n",runner.title_ticks);
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
    assert(DsiFileSelectProbe_Prepare());
    assert(gDsiFileSelectProbeStats.prepared_slots == 3);
    printf("PASS E16: Original A -> file select after %u title ticks\n",runner.title_ticks);
    /* Guard must reject accidental execution outside task 1. */
    gMain.task = TASK_GAME;
    assert(!DsiFileSelectPalette_Step());
    assert(!DsiFileSelectProbe_Prepare());
    assert(gDsiFileSelectProbeStats.errors == 1);
    assert(!DsiFileSelectProbe_LoadIntroAssets());
    assert(!DsiFileSelectProbe_TestOptions());
    assert(gDsiFileSelectProbeStats.errors == 3);
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
    puts("PASS E16: unported task and title-frame budget are safely rejected");
    DsiGbaBus_DetachRom();
    TmcRom_Close(&rom);
    return 0;
}
