/* DSi native ENGINE E18. Integration test, NOT a playable port.
 * Real source modules: src/common.c, src/main.c, src/flags.c, src/title.c.
 * Real functions invoked: ReadKeyInput() and SetTask().
 * Native bridge: GBA memory shadows and paged SD ROM reads.
 * AgbMain() intentionally NOT called; first 3 TitleTask ticks load ROM
 * graphics/palettes to a GBA memory shadow. E9 advances the ORIGINAL logo
 * timer and animates original BGs and title OAM sprites.
 * E11 additionally executes the original titleScreenObject.c entity states.
 * E13 routes A/START through Zelda, stopping safely at TASK_FILE_SELECT. */
/* libnds and The Minish Cap both define a different struct LinkedList.
 * Change only libnds' spelling while importing its headers. This does NOT
 * rename or modify Zelda's original entity lists or any engine source. */
#define LinkedList DsiLibndsLinkedList
#include <nds.h>
#include <fat.h>
#undef LinkedList
#include <stdbool.h>
#include <stdint.h>
#include <stdio.h>
#include <string.h>
#include "common.h"
#include "main.h"
#include "game.h"
#include "area.h"
#include "flags.h"
#include "area.h"
#include "flags.h"
#include "dsi_gba_bus.h"
#include "dsi_rom_refs.h"
#include "dsi_engine_state.h"
#include "dsi_title_trace.h"
#include "dsi_title_assets.h"
#include "dsi_gba_renderer.h"
#include "dsi_title_composite.h"
#include "dsi_title_live.h"
#include "dsi_title_sprites.h"
#include "dsi_title_entities.h"
#include "dsi_title_object_gfx.h"
#include "dsi_title_runner.h"
#include "dsi_file_select_probe.h"
#include "dsi_file_select_palette.h"
#include "dsi_game_bootstrap.h"
#include "dsi_game_transition.h"
#include "dsi_game_native.h"
#include "dsi_room_probe.h"
#include "dsi_room_maps.h"
#include "dsi_room_tiles.h"
#include "dsi_room_preview.h"
#include "dsi_room_resources.h"
#include "dsi_entity_lists.h"
#include "dsi_area_headers.h"
#include "port_gba_mem.h"

Input gInput;
Main gMain;
static PrintConsole bottom;
static TmcRomReader reader;
static bool romReady;
/* 256x192x2 = 96 KiB DSi main-screen RGB555 bitmap. */
static uint16_t mainFrame[DSI_RENDER_W * DSI_RENDER_H] __attribute__((aligned(4)));
static DsiRenderStats renderStats;
static DsiTitleLive live;

static bool find_rom(void) {
    static const char *paths[] = {
        "fat:/tmc-dsi/baserom_eu.gba", "fat:/tmc-dsi/baserom.gba",
        "sd:/tmc-dsi/baserom_eu.gba", "sd:/tmc-dsi/baserom.gba"
    };
    for (size_t i=0; i<sizeof(paths)/sizeof(paths[0]); ++i) {
        if (TmcRom_Open(&reader, paths[i])) {
            DsiGbaBus_AttachRom(&reader);
            return true;
        }
    }
    return false;
}

static bool memory_test(void) {
    gba_write8(0x02000041u, 0x5Au);
    gba_write16(0x03000060u, 0x1234u);
    gba_write32(0x06000020u, 0xBEEFCAFEu);
    gba_write16(0x05000006u, 0x3210u);
    return gba_read8(0x02000041u) == 0x5Au &&
           gba_read16(0x03000060u) == 0x1234u &&
           gba_read32(0x06000020u) == 0xBEEFCAFEu &&
           gba_read16(0x05000006u) == 0x3210u;
}
/* Read the real compiled decompilation metadata on DSi.
 * Keeping it live also ensures the linker retains these original tables. */
static bool original_area_metadata_test(void) {
    const AreaHeader *house = &gAreaMetadata[AREA_HOUSE_INTERIORS_2];
    return house->flag_bank == LOCAL_BANK_2 &&
           (house->flags & AR_HAS_ENEMIES) != 0 &&
           gOverworldLocations[0].minX == 0 &&
           gOverworldLocations[17].minX == 0xFFu;
}
static bool engine_task_test(void) {
    /* Calls the ORIGINAL implementation in src/main.c. This does not
     * call TitleTask or AgbMain and does not simulate gameplay. */
    memset(&gMain, 0, sizeof(gMain));
    SetTask(TASK_GAME);
    if (gMain.task != TASK_GAME || gMain.state != GAMETASK_TRANSITION ||
        gMain.substate != GAMEMAIN_INITROOM) return false;
    SetTask(TASK_TITLE);
    return gMain.task == TASK_TITLE && gMain.state == GAMETASK_TRANSITION &&
           gMain.substate == GAMEMAIN_INITROOM;
}
static bool rom_bus_test(void) {
    uint8_t id[4] = {0};
    uint8_t raw[20], bridged[20];
    /* ROM header game code is at 0xAC; crossing cache page boundary 0x1000. */
    if (!DsiGbaBus_Read(0x080000ACu, id, sizeof(id))) return false;
    if (memcmp(id, "BZMP", 4) != 0 && memcmp(id, "BZME", 4) != 0) return false;
    if (!TmcRom_Read(&reader, 0xFFAu, raw, sizeof(raw))) return false;
    if (!DsiGbaBus_Read(0x08000FFAu, bridged, sizeof(bridged))) return false;
    return memcmp(raw, bridged, sizeof(raw)) == 0;
}

int main(void) {
    videoSetMode(MODE_5_2D);
    vramSetBankA(VRAM_A_MAIN_BG);
    int mainBg = bgInit(2, BgType_Bmp16, BgSize_B16_256x256, 0, 0);
    bgSetPriority(mainBg, 0);
    memset(bgGetGfxPtr(mainBg), 0, 256u * 256u * sizeof(uint16_t));
    videoSetModeSub(MODE_0_2D);
    vramSetBankC(VRAM_C_SUB_BG);
    consoleInit(&bottom, 0, BgType_Text4bpp, BgSize_T_256x256, 31, 0, false, true);
    consoleSelect(&bottom);
    DsiGbaBus_Init();
    DsiRoomResources_Reset();
    DsiEntityLists_Reset();
    DsiAreaHeaders_Reset();
    bool flags_ok = DsiEngineState_SelfTest();
    iprintf("TMC / DSi ENGINE E28\n");
    iprintf("========================\n");
    iprintf("DSi mode: %s\n", isDSiMode()?"YES":"NO");
    iprintf("GBA memory: %s\n", memory_test()?"PASS":"FAIL");
    iprintf("SetTask original: %s\n", engine_task_test()?"PASS":"FAIL");
    iprintf("Flags original: %s\n", flags_ok?"PASS":"FAIL");
    iprintf("Area tables: %s\n", original_area_metadata_test()?"PASS":"FAIL");
    bool storage = fatInitDefault();
    iprintf("SD/FAT: %s\n", storage?"ready":"FAILED");
    romReady = storage && find_rom();
    iprintf("ROM: %s\n", !romReady?"missing":
        reader.region == TMC_ROM_REGION_EUROPE?"EU":"USA");
    if (romReady && reader.region != TMC_ROM_REGION_EUROPE) {
        iprintf("EU ROM REQUIRED for E16!\n");
        DsiGbaBus_DetachRom();
        TmcRom_Close(&reader);
        romReady = false;
    }
    iprintf("ROM bus: %s\n", romReady ?
        (rom_bus_test()?"PASS":"FAIL") : "not tested");
    uint32_t first_word = 0;
    bool rom_word_ok = romReady && DsiRomRefs_ReadU32(0x080000ACu, &first_word) && first_word == 0x504D5A42u;
    iprintf("ROM refs: %s\n", rom_word_ok?"PASS":"NOT READY");
    DsiTitleAssets_Enable(romReady, RegionDefaultLanguage());
    DsiTitleSprites_Enable(romReady);
    DsiTitleEntities_Enable(romReady);
    DsiTitleAssets_ResetStats();
    bool title_trace_ok = romReady && DsiTitleTrace_TestFirstTransitions();
    iprintf("TitleTask: %s\n", title_trace_ok ? "PASS (3 steps)" : "NOT READY");
    bool assets_ok = title_trace_ok && gDsiTitleAssetStats.errors == 0 &&
        gDsiTitleAssetStats.gfx_groups == 2 && gDsiTitleAssetStats.palette_groups == 1;
    iprintf("ROM assets: %s\n", assets_ok ? "PASS" : "NOT READY");
    bool render_ok = title_trace_ok && assets_ok &&
        (gScreen.lcd.displayControl & DISPCNT_BG2_ON) &&
        DsiGbaRenderer_DrawTextBackground(mainFrame, DSI_RENDER_W,
            gVram, sizeof(gVram), gBgPltt, 256,
            gScreen.bg2.control, gScreen.bg2.xOffset,
            gScreen.bg2.yOffset, &renderStats);
    if (render_ok) {
        /* On DS, 16-bpp main BG bitmap is 256x256, bank A = 128 KiB.
         * Copy the visible 192 rows after finishing all ROM reads. */
        dmaCopy(mainFrame, bgGetGfxPtr(mainBg), sizeof(mainFrame));
    }
    iprintf("Renderer: %s\n", render_ok ? "VISIBLE" : "NOT READY");
    iprintf("Title stage: %s\n", render_ok ? "LOGOS" : "NOT READY");
    iprintf("Logo pixels: %lu\n", (unsigned long)renderStats.pixels_not_backdrop);
    iprintf("AgbMain: NOT RUNNING\n");
    iprintf("ReadKeyInput ORIGINAL\n");
    unsigned long frames = 0;
    DsiTitleRunner title_runner;
    DsiTitleRunner_Init(&title_runner);
    bool handoff_reported = false;
    bool forest_ready = false;
    bool title_fault = false;
    bool file_palette_live = false;
    DsiTitleLive_Init(&live);
    DsiGameBootstrap_Reset();
    DsiGameNative_Reset();
    bool file_select_ram_ready = false;
    bool game_bootstrap_done = false;
    /* E13 permits ORIGINAL A/START->File Select transition, but will NEVER
     * invoke FileSelectTask until that module is safely integrated. */
    while (pmMainLoop()) {
        swiWaitForVBlank();
        scanKeys();
        uint16_t gbaKeys = (uint16_t)(~keysHeld()) & 0x03FFu;
        gba_write16(0x04000130u, gbaKeys);
        ReadKeyInput();
        /* SELECT exits diagnostics. START must reach Zelda's TitleTask. */
        if (gInput.newKeys & 0x04u) break;
        if (render_ok && !title_fault && !handoff_reported &&
            title_runner.result == DSI_TITLE_RUNNING) {
            DsiTitleRunResult stage = DsiTitleRunner_Step(&title_runner);
            if (stage != DSI_TITLE_RUNNING) {
                handoff_reported = true;
                if (stage == DSI_TITLE_AT_FILE_SELECT) {
                    bool prepared = DsiFileSelectProbe_Prepare();
                    bool options = prepared && DsiFileSelectProbe_TestOptions();
                    bool assets = options && DsiFileSelectProbe_LoadIntroAssets();
                    bool localized = assets && DsiFileSelectProbe_LoadLanguageAssets();
                    file_palette_live = localized && DsiFileSelectPalette_Initialize();
                    file_select_ram_ready = prepared && options;
                    iprintf("\x1b[16;0HFile-select: %s  ", localized ? "GFX READY" : "BLOCKED");
                    if (localized) {
                        iprintf("\x1b[17;0Hgfx:%u pal:%u lang:%u ",
                                gDsiFileSelectProbeStats.loaded_gfx_bytes,
                                gDsiFileSelectProbeStats.loaded_pal_bytes,
                                gDsiFileSelectProbeStats.language_gfx_bytes);
                    }
                }
                else
                    iprintf("\x1b[16;0HTitle stop: %u      ", (unsigned)stage);
            }
            bool changed = false;
            if (!DsiTitleLive_Render(&live, mainFrame, DSI_RENDER_W, &changed) ||
                gDsiTitleEntityStats.errors != 0) {
                title_fault = true;
                iprintf("\x1b[16;0HTitle render: ERROR ");
            } else if (changed) {
                dmaCopy(mainFrame, bgGetGfxPtr(mainBg), sizeof(mainFrame));
                if (!forest_ready) {
                    forest_ready = true;
                    iprintf("\x1b[13;0HTitle stage: FOREST  ");
                    iprintf("\x1b[14;0HBG blend: %lu px    ", (unsigned long)live.stats.blended_pixels);
                }
            }
        }
        /* E17 can advance the genuine menu palette timer without entering
         * the unsafe original FileSelectTask / save-file handlers. */
        /* E18 opt-in: X selects an original RAM-only slot and calls the
         * unmodified SetTask(TASK_GAME), but NEVER executes GameTask.
         * This is a safe handoff diagnostic, not a playable scene. */
        /* E25: automatically continue from the ORIGINAL title handoff into
         * the RAM-only game bootstrap. X still works as manual fallback. */
        if (file_select_ram_ready && !game_bootstrap_done &&
            gMain.task == TASK_FILE_SELECT) {
            game_bootstrap_done = DsiGameBootstrap_Begin(0);
            if (game_bootstrap_done) {
                file_palette_live = false;
                iprintf("\x1b[15;0HGame RAM: READY       ");
                bool original_game_boot = DsiGameNative_BootTwoStates();
                const DsiGameNativeStats *native = DsiGameNative_Status();
                /* E24 performs the ORIGINAL GameMain_InitRoom prefix only
                 * after the safe room-ROM/map/tiles preflights below. */
                /* E20: read ONLY original EU ROM room metadata. The room
                 * coordinate is derived from the E19 original GameTask host
                 * trace, NOT from a running native GameTask/InitRoom. */
                DsiRoomMetadata room_metadata = {0};
                bool room_ok = original_game_boot && romReady &&
                    DsiRoomProbe_ReadEU(0x22u, 0x15u, &room_metadata);
                /* E27.2: pin original map/tileset commands as stable data.
                 * Does not dispatch InitRoom or change E25 rendering. */
                bool room_refs_ok = room_ok &&
                    DsiRoomResources_PrepareEU(&room_metadata);
                /* E28.3: exact 16-byte ORIGINAL EU entity records are
                 * preserved in native RAM for future InitializeEntities.
                 * This prepares data only; no entities are spawned. */
                bool entity_data_ok = room_ok &&
                    DsiEntityLists_PrepareEU(&room_metadata);
                /* Original EU room header table: all native copies survive
                 * ROM cache eviction. Compare to the already verified current
                 * room before publishing the diagnostic. */
                bool area_hdr_ok = room_ok && DsiAreaHeaders_PrepareEU(room_metadata.area);
                const RoomHeader *current_hdr = area_hdr_ok ?
                    DsiAreaHeaders_Get(room_metadata.area, room_metadata.room) : NULL;
                area_hdr_ok = current_hdr != NULL &&
                    current_hdr->map_x == room_metadata.map_x &&
                    current_hdr->map_y == room_metadata.map_y &&
                    current_hdr->pixel_width == room_metadata.width &&
                    current_hdr->pixel_height == room_metadata.height &&
                    current_hdr->tileSet_id == room_metadata.tileset_id;
                iprintf("\x1b[8;0HRoom cache: %-7s H:%s",
                        room_refs_ok ? "READY" : "PENDING",
                        area_hdr_ok ? "OK" : "ERR");
                DsiRoomMapStats map_stats = {0};
                bool map_ok = room_ok && DsiRoomMaps_LoadEU(&room_metadata, &map_stats);
                DsiRoomTileStats tile_stats = {0};
                /* These are the ORIGINAL packed ROM tileSet/tiles commands,
                 * transferred to GBA shadow VRAM/RAM, not a new room engine. */
                bool tile_ok = map_ok && DsiRoomTiles_LoadEU(&room_metadata, &tile_stats);
                bool prefix_ok = tile_ok && DsiGameNative_BootRoomPrefix();
                /* E25: turn ROM-original room metatiles into a DS bitmap.
                 * This is NOT InitRoom(), Link, entities or playable game. */
                DsiRoomPreviewStats room_draw_stats = {0};
                bool scene_ok = false;
                if (prefix_ok) {
                    /* Renderer can partially update its buffer on failure;
                     * never display it until a complete valid render. */
                    scene_ok = DsiRoomPreview_Draw(&room_metadata, mainFrame,
                                                   DSI_RENDER_W, &room_draw_stats);
                    if (scene_ok) {
                        dmaCopy(mainFrame, bgGetGfxPtr(mainBg), sizeof(mainFrame));
                        iprintf("\x1b[9;0HROOM SCREEN: DISPLAYED    ");
                    } else {
                        iprintf("\x1b[9;0HROOM SCREEN: BLOCKED      ");
                    }
                }
                native = DsiGameNative_Status();
                /* E28: Only the actual original InitRoom()/InitAllRoomResInfo()
                 * can fill gArea after the E24 transition. Verify the active
                 * room pointer and metadata, not an independent imitation. */
                const RoomResInfo *initialized = gArea.pCurrentRoomInfo;
                bool original_room_ok = prefix_ok && room_refs_ok && area_hdr_ok &&
                    gRoomControls.room < MAX_ROOMS &&
                    initialized == &gArea.roomResInfos[gRoomControls.room] &&
                    gArea.flag_bank == gAreaMetadata[gRoomControls.area].flag_bank &&
                    gArea.localFlagOffset == gLocalFlagBanks[gArea.flag_bank] &&
                    gArea.portal_timer == 180u && gArea.lightLevel == 256u &&
                    initialized->pixel_width == room_metadata.width &&
                    initialized->pixel_height == room_metadata.height &&
                    initialized->map_x == room_metadata.map_x &&
                    initialized->map_y == room_metadata.map_y &&
                    initialized->tileSet != NULL && initialized->map != NULL &&
                    initialized->tiles != NULL;
                iprintf("\x1b[10;0HInitRoom original: %-6s",
                        original_room_ok ? "PASS" : "PENDING");
                iprintf("\x1b[16;0HGameTask: %-11s", prefix_ok ? "ROOM PREFIX" : "BLOCKED");
                iprintf("\x1b[17;0Horg:%u/%u/%u init:%u d:%u e:%u  ",
                        native->transition_ticks, native->init_ticks,
                        native->room_prefix_ticks, native->original_room_calls,
                        native->deferred_services, native->errors);
                iprintf("\x1b[11;0HRoom tiles: %s %lu B   ", tile_ok ? "OK" : "BLOCKED",
                        (unsigned long)(tile_stats.tileset.uncompressed_bytes +
                                        tile_stats.tiles.uncompressed_bytes));
                iprintf("\x1b[12;0HRoom ROM: %s       ", room_ok ? "VERIFIED" : "BLOCKED");
                if (room_ok) {
                    iprintf("\x1b[13;0Hsize: %ux%u area:22 room:15   ",
                            room_metadata.width, room_metadata.height);
                    iprintf("\x1b[14;0Hentity lists: %u + %u    ",
                            room_metadata.entity_counts[0], room_metadata.entity_counts[1]);
                }
                if (scene_ok) {
                    iprintf("\x1b[14;0HRoom pixels:%lu E:%s  ",
                            (unsigned long)room_draw_stats.colored_pixels,
                            entity_data_ok ? "READY" : "BAD");
                }
            } else {
                iprintf("\x1b[15;0HGame RAM: BLOCKED     ");
            }
        }
        if (file_palette_live && !DsiFileSelectPalette_Step()) {
            file_palette_live = false;
            iprintf("\x1b[16;0HMenu palettes: FAILED  ");
        }
        /* E23.1: redraw each diagnostic on its OWN fixed console row.
         * The old code started at row 18, then printed 8 lines with \n.
         * libnds console only has 24 rows: that scrolled away the actual
         * GameTask and room results every 8 frames. Never print a newline
         * in this live block, and keep all output within rows 18..23. */
        if ((++frames % 8) == 0) {
            char line[64];
            snprintf(line, sizeof(line), "H:%03X N:%03X task:%u s:%u",
                     gInput.heldKeys, gInput.newKeys, gMain.task, gMain.state);
            iprintf("\x1b[18;0H%-31.31s", line);
            snprintf(line, sizeof(line), "frames:%lu draws:%u", frames, live.draws);
            iprintf("\x1b[19;0H%-31.31s", line);
            snprintf(line, sizeof(line), "pal:%u alpha:%u",
                     live.palette_updates, live.alpha_updates);
            iprintf("\x1b[20;0H%-31.31s", line);
            snprintf(line, sizeof(line), "OAM:%u sprites:%u",
                     DsiTitleSprites_Count(), gDsiTitleSpriteStats.draw_calls);
            iprintf("\x1b[21;0H%-31.31s", line);
            snprintf(line, sizeof(line), "entity:%u upd:%u err:%u",
                     gDsiTitleEntityStats.alive, gDsiTitleEntityStats.updated,
                     gDsiTitleEntityStats.errors);
            iprintf("\x1b[22;0H%-31.31s", line);
            snprintf(line, sizeof(line), "OBJgfx:%u pal:%u err:%u",
                     gDsiTitleObjectGfxStats.gfx_loaded,
                     gDsiTitleObjectGfxStats.pal_loaded,
                     gDsiTitleObjectGfxStats.errors);
            iprintf("\x1b[23;0H%-31.31s", line);
            if (file_palette_live)
                iprintf("\x1b[15;0HMenu pal ticks:%-6u", gDsiFileSelectPaletteTicks);
        }
    }
    DsiGbaBus_DetachRom();
    if (romReady) TmcRom_Close(&reader);
    return 0;
}
