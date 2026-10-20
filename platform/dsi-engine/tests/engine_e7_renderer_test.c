/* E7 verification: original src/title.c drives the engine's own BG2 regs;
 * a regular GBA tile renderer turns ROM-loaded VRAM into DS 16-bit pixels. */
#include "dsi_gba_renderer.h"
#include "dsi_title_assets.h"
#include "dsi_title_trace.h"
#include "dsi_gba_bus.h"
#include "tmc_rom_stream.h"
#include "port_gba_mem.h"
#include "main.h"
#include "common.h"
#include "screen.h"
#include <assert.h>
#include <stdio.h>
#include <string.h>
#include <stdlib.h>

Input gInput;
Main gMain;
static uint16_t frame[DSI_RENDER_W * DSI_RENDER_H];
static uint32_t hash_visible(void) {
    uint32_t hash = 2166136261u;
    for (int y = 16; y < 176; ++y) for (int x = 8; x < 248; ++x) {
        uint16_t px = frame[y * DSI_RENDER_W + x];
        hash = (hash ^ (px & 255u)) * 16777619u;
        hash = (hash ^ (px >> 8)) * 16777619u;
    }
    return hash;
}
int main(int argc, char **argv) {
    assert(argc >= 2);
    DsiGbaBus_Init();
    gBgPltt[0] = 0x7fff;
    gBgPltt[50] = 0x03e0; /* 4bpp palette bank 3, color 2 */
    gVram[1 * 32] = 0x02;
    gVram[0x800] = 1;
    gVram[0x801] = 0x30; /* BG map entry: tile 1, palette bank 3 */
    DsiRenderStats st;
    assert(DsiGbaRenderer_DrawTextBackground(frame, DSI_RENDER_W,
         gVram, sizeof(gVram), gBgPltt, 256, (1u << 8), 0, 0, &st));
    assert(frame[16 * DSI_RENDER_W + 8] == (0x8000 | 0x03e0));
    assert(frame[16 * DSI_RENDER_W + 9] == (0x8000 | 0x7fff));
    assert(st.pixels_drawn == 240 * 160 && st.pixels_not_backdrop == 1);
    /* horizontal flip mirrors that first colored pixel onto column seven */
    gVram[0x801] = 0x34;
    assert(DsiGbaRenderer_DrawTextBackground(frame, DSI_RENDER_W,
         gVram, sizeof(gVram), gBgPltt, 256, (1u << 8), 0, 0, &st));
    assert(frame[16 * DSI_RENDER_W + 15] == (0x8000 | 0x03e0));
    assert(frame[16 * DSI_RENDER_W + 8] == (0x8000 | 0x7fff));
    assert(frame[0] == (0x8000 | 0x7fff));
    /* Bounds defense against a bad screen base in a future engine state. */
    assert(!DsiGbaRenderer_DrawTextBackground(frame, DSI_RENDER_W,
        gVram, 0x10000, gBgPltt, 256, (31u << 8) | (1u << 14), 0, 0, &st));
    puts("PASS E7: GBA tile decode, palette bank, transparent pixel, H-flip, letterbox, bounds");

    DsiGbaBus_Init();
    TmcRomReader rom = {0};
    assert(TmcRom_Open(&rom, argv[1]));
    assert(rom.region == TMC_ROM_REGION_EUROPE);
    DsiGbaBus_AttachRom(&rom);
    DsiTitleAssets_Enable(true, 1);
    DsiTitleAssets_ResetStats();
    assert(DsiTitleTrace_TestFirstTransitions());
    assert(gDsiTitleAssetStats.errors == 0);
    assert(gScreen.bg2.control == 0x1D02); /* original ResetScreenRegs() */
    assert(gScreen.lcd.displayControl & 0x400); /* original TitleTask sets BG2 enable */
    assert(DsiGbaRenderer_DrawTextBackground(frame, DSI_RENDER_W,
        gVram, sizeof(gVram), gBgPltt, 256,
        gScreen.bg2.control, gScreen.bg2.xOffset, gScreen.bg2.yOffset, &st));
    assert(st.pixels_not_backdrop == 4981);
    /* Golden visible-frame FNV1a, independently cross-checked against a
     * tilemap/palette decoder of the same EU ROM's raw transfer records. */
    assert(hash_visible() == 0x16366349u);
    assert(st.tiles_used > 20);
    printf("PASS E7: original TitleTask + ResetScreenRegs produced visible BG2 Nintendo/Capcom logo pixels=%u tiles=%u frame_hash=%08x\n",
        (unsigned)st.pixels_not_backdrop, (unsigned)st.tiles_used, (unsigned)hash_visible());
    if (argc >= 3) {
        FILE *f = fopen(argv[2], "wb");
        assert(f);
        fprintf(f, "P6\n256 192\n255\n");
        for (int y=0; y < 192; ++y) for (int x=0; x<256; ++x) {
            uint16_t c=frame[y*256+x];
            uint8_t rgb[3] = { (uint8_t)((c&31)*255/31),
                (uint8_t)(((c>>5)&31)*255/31),
                (uint8_t)(((c>>10)&31)*255/31) };
            assert(fwrite(rgb, 1, 3, f)==3);
        }
        fclose(f);
    }
    DsiGbaBus_DetachRom();
    TmcRom_Close(&rom);
    puts("NOT YET: frame timing, fades, sprites, complete AgbMain and playable title");
    return 0;
}
