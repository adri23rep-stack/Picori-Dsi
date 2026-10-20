#ifndef TMC_GFX_PREVIEW_H
#define TMC_GFX_PREVIEW_H

/* ROM-backed, standalone title-tile preview. NOT the game renderer.
 * GBA uses 8x8, 4bpp tiles and 16-color palettes. All offsets are literal
 * file offsets, taken from this source project's USA / EU asset tables.
 */
#include <stdbool.h>
#include <stdint.h>
#include "tmc_rom_stream.h"

#define TMC_TITLE_TILE_BYTES 32768u
#define TMC_TITLE_TILEMAP_SIDE 256u
#define TMC_TITLE_PALETTE_COUNT 15u

/* ~32.5 KiB stored in static RAM. No game graphics are shipped in the build. */
typedef struct TmcGfxPreview {
    uint8_t tiles[TMC_TITLE_TILE_BYTES];
    uint16_t palette[TMC_TITLE_PALETTE_COUNT][16];
    bool ready;
} TmcGfxPreview;

bool TmcGfxPreview_Load(TmcGfxPreview *preview, TmcRomReader *reader);
/* Returns an opaque DS BGR555 pixel. Caller scrolls/crops as desired. */
uint16_t TmcGfxPreview_Pixel(const TmcGfxPreview *preview,
                             unsigned x, unsigned y, unsigned palette_index);

#endif
