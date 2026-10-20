/* E25 ROM integration smoke test (runs on PC).
 * Does NOT launch gameplay; reads original EU source ROM through the same
 * DSi paged-ROM bridge and writes a 256x192 PPM of the first room preview.
 */
#include "dsi_gba_bus.h"
#include "dsi_room_probe.h"
#include "dsi_room_maps.h"
#include "dsi_room_tiles.h"
#include "dsi_room_preview.h"
#include "dsi_title_assets.h"
#include "tmc_rom_stream.h"
#include <stdio.h>
#include <stdint.h>
#include <stdlib.h>

static uint16_t image[192][256];
static int write_ppm(const char *name) {
    FILE *f = fopen(name, "wb");
    if (!f) return 0;
    fprintf(f, "P6\n256 192\n255\n");
    for (unsigned y=0; y<192; ++y) for (unsigned x=0; x<256; ++x) {
        const uint16_t pix=image[y][x];
        unsigned char rgb[3] = {
            (unsigned char)((pix & 31u) * 255u / 31u),
            (unsigned char)(((pix >> 5) & 31u) * 255u / 31u),
            (unsigned char)(((pix >> 10) & 31u) * 255u / 31u)
        };
        if (fwrite(rgb, 1, 3, f) != 3) { fclose(f); return 0; }
    }
    return fclose(f) == 0;
}
int main(int argc, char **argv) {
    if (argc < 2 || argc > 3) {
        fputs("Usage: e25_rom_preview_test baserom_eu.gba [preview.ppm]\n",stderr);
        return 2;
    }
    TmcRomReader r={0};
    DsiGbaBus_Init();
    if (!TmcRom_Open(&r, argv[1]) || r.region != TMC_ROM_REGION_EUROPE) {
        fputs("E25: need the UNMODIFIED EU GBA ROM (no ROM bundled).\n",stderr);
        return 3;
    }
    DsiGbaBus_AttachRom(&r);
    DsiTitleAssets_Enable(true, 2);
    DsiTitleAssets_ResetStats();
    DsiRoomMetadata room={0};
    DsiRoomMapStats maps={0};
    DsiRoomTileStats tiles={0};
    DsiRoomPreviewStats stats={0};
    if (!DsiRoomProbe_ReadEU(0x22, 0x15, &room) ||
        !DsiRoomMaps_LoadEU(&room, &maps) ||
        !DsiRoomTiles_LoadEU(&room, &tiles) ||
        !DsiRoomPreview_Draw(&room, &image[0][0], 256, &stats)) {
        fputs("E25: ROM assets / preview failed, cannot claim scene displayed.\n",stderr);
        DsiGbaBus_DetachRom(); TmcRom_Close(&r);
        return 4;
    }
    printf("E25 EU ROM room: %u x %u pixels, %lu loaded bytes, %lu non-backdrop pixels, %lu special tiles skipped\n",
           room.width,room.height,
           (unsigned long)(maps.uncompressed_bytes + tiles.tileset.uncompressed_bytes + tiles.tiles.uncompressed_bytes),
           (unsigned long)stats.colored_pixels, (unsigned long)stats.skipped_special);
    int ok=(stats.colored_pixels > 0u);
    if (ok && argc == 3) ok=write_ppm(argv[2]);
    DsiGbaBus_DetachRom(); TmcRom_Close(&r);
    return ok ? 0 : 5;
}
