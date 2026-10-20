#include "tmc_gfx_preview.h"
#include <string.h>

/* Traced to assets/gfx.json and build/EU/assets/gfx_offsets.h:
 * EU : gGlobalGfxAndPalettes at 0x005A23D0, tile offset + 0x00322360,
 *      palette group 4 starts at + 0x0001AA20.
 * USA: assets/gfx.json absolute start addresses. Unverified with a USA ROM.
 * Group 2: gGfx_2_0.4bpp; group 4 palettes 0..14: gPalette_3411..3425.
 * These are a tile SHEET, not a composited GBA title screen or tilemap.
 */
#define EU_TILE_ROM_OFFSET 0x008C4730u
#define EU_PALETTE_ROM_OFFSET 0x005BCDF0u
#define USA_TILE_ROM_OFFSET 0x008C4F60u
#define USA_PALETTE_ROM_OFFSET 0x005BD8E0u

bool TmcGfxPreview_Load(TmcGfxPreview *preview, TmcRomReader *reader) {
    uint32_t tile_offset, palette_offset;
    uint8_t raw_palettes[TMC_TITLE_PALETTE_COUNT * 16u * 2u];
    unsigned i, j;
    if (!preview || !reader || !reader->file) return false;
    preview->ready = false;
    switch (reader->region) {
        case TMC_ROM_REGION_EUROPE:
            tile_offset = EU_TILE_ROM_OFFSET;
            palette_offset = EU_PALETTE_ROM_OFFSET;
            break;
        case TMC_ROM_REGION_USA:
            tile_offset = USA_TILE_ROM_OFFSET;
            palette_offset = USA_PALETTE_ROM_OFFSET;
            break;
        default: return false;
    }
    if (!TmcRom_Read(reader, tile_offset, preview->tiles,
                     sizeof(preview->tiles)) ||
        !TmcRom_Read(reader, palette_offset, raw_palettes,
                     sizeof(raw_palettes))) {
        return false;
    }
    for (i = 0; i < TMC_TITLE_PALETTE_COUNT; ++i) {
        for (j = 0; j < 16; ++j) {
            unsigned k = (i * 16u + j) * 2u;
            preview->palette[i][j] = (uint16_t)(raw_palettes[k] |
                                                ((uint16_t)raw_palettes[k + 1] << 8));
        }
    }
    preview->ready = true;
    return true;
}

uint16_t TmcGfxPreview_Pixel(const TmcGfxPreview *preview,
                             unsigned x, unsigned y, unsigned palette_index) {
    if (!preview || !preview->ready ||
        x >= TMC_TITLE_TILEMAP_SIDE || y >= TMC_TITLE_TILEMAP_SIDE ||
        palette_index >= TMC_TITLE_PALETTE_COUNT) return 0x8000u;
    /* 32 tiles per row, each GBA 4bpp tile occupies exactly 32 bytes. */
    unsigned tile = (y >> 3) * 32u + (x >> 3);
    unsigned byte = tile * 32u + (y & 7u) * 4u + ((x & 7u) >> 1);
    uint8_t packed = preview->tiles[byte];
    unsigned color = (packed >> ((x & 1u) ? 4u : 0u)) & 15u;
    return (uint16_t)((preview->palette[palette_index][color] & 0x7FFFu) | 0x8000u);
}
