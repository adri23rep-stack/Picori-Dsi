/* E6 host test: originals title/main/common plus actual EU-ROM resource transfer. */
#include "dsi_title_assets.h"
#include "dsi_title_trace.h"
#include "dsi_gba_bus.h"
#include "tmc_rom_stream.h"
#include "main.h"
#include "common.h"
#include <assert.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
Input gInput;
Main gMain;
static void assert_from_rom(TmcRomReader *r, uint32_t rom_offset, uint32_t bus_dest, size_t n) {
    uint8_t from_rom[8192], from_shadow[8192];
    assert(n <= sizeof(from_rom));
    assert(TmcRom_Read(r, rom_offset, from_rom, n));
    assert(DsiGbaBus_Read(bus_dest, from_shadow, n));
    assert(!memcmp(from_rom, from_shadow, n));
}
int main(int argc, char **argv) {
    assert(argc == 3);
    TmcRomReader rom = {0};
    DsiGbaBus_Init();
    assert(TmcRom_Open(&rom, argv[1]));
    assert(rom.region == TMC_ROM_REGION_EUROPE);
    DsiGbaBus_AttachRom(&rom);
    DsiTitleAssets_Enable(true, 1);
    DsiTitleAssets_ResetStats();
    assert(DsiTitleTrace_TestFirstTransitions());
    assert(gDsiTitleAssetStats.errors == 0);
    assert(gDsiTitleAssetStats.gfx_groups == 2 && gDsiTitleAssetStats.gfx_records == 4);
    assert(gDsiTitleAssetStats.palette_groups == 1 && gDsiTitleAssetStats.palette_records == 2);
    assert(gDsiTitleAssetStats.bytes_gfx == (0x1C0+0xE00+0xF60+0x500));
    assert(gDsiTitleAssetStats.bytes_pal == 96);
    /* These values come from original EU gfx/pal group descriptors. */
    assert_from_rom(&rom, 0x5A23D0+0x0215A0, 0x06012000, 0x1c0);
    assert_from_rom(&rom, 0x5A23D0+0x0207A0, 0x0600C000, 0xE00);
    assert_from_rom(&rom, 0x5A23D0+0x31F680, 0x06000000, 0xF60);
    assert_from_rom(&rom, 0x5A23D0+0x3205E0, 0x0600E800, 0x500);
    assert_from_rom(&rom, 0x5A23D0+3388*32, 0x05000000, 64);
    assert_from_rom(&rom, 0x5A23D0+12*32, 0x050001e0, 32);
    assert(!DsiTitleAssets_LoadGfxGroup(133));
    assert(!DsiTitleAssets_LoadPaletteGroup(207));
    DsiGbaBus_DetachRom();
    TmcRom_Close(&rom);

    /* Synthetic image: compressed ABCDABCDABCD using literal+backreference. */
    DsiGbaBus_Init();
    assert(TmcRom_Open(&rom, argv[2]));
    DsiGbaBus_AttachRom(&rom);
    uint32_t n = 0;
    assert(DsiTitleAssets_UnpackLz77(0x08000300u, 0x06010000u, &n));
    assert(n == 12);
    char expanded[12];
    assert(DsiGbaBus_Read(0x06010000u, expanded, sizeof(expanded)));
    assert(memcmp(expanded, "ABCDABCDABCD", 12) == 0);
    assert(!DsiTitleAssets_UnpackLz77(0x08000300u, 0x06017FFBu, &n)); /* range check */
    DsiGbaBus_DetachRom();
    TmcRom_Close(&rom);
    puts("PASS: Original TitleTask loaded EU title GFX 16,1 and PAL 2 (byte-perfect comparison)");
    puts("PASS: LZ77 literal/backreference, destination bounds, invalid group rejection");
    puts("NOT YET: real GBA screen renderer / AgbMain / audio");
    return 0;
}
