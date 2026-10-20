#include "dsi_gba_renderer.h"
#include <string.h>

/* GBA regular tiled backgrounds: 32x32-tile screenblocks (2 KiB each),
 * character banks of 16 KiB, 4/8 bits per pixel, and flip/palette map bits.
 * Unlike a graphic-sheet viewer we follow BG control + map from the engine. */
bool DsiGbaRenderer_DrawTextBackground(uint16_t *out, size_t stride,
    const uint8_t *vram, size_t vram_length, const uint16_t *palette,
    size_t palette_entries, uint16_t control, int16_t scroll_x,
    int16_t scroll_y, DsiRenderStats *stats) {
    if (stats) memset(stats, 0, sizeof(*stats));
    if (!out || !vram || !palette || stride < DSI_RENDER_W ||
        vram_length < 0x10000 || palette_entries < 256) return false;

    const unsigned char_bank = (control >> 2) & 3u;
    const unsigned screen_bank = (control >> 8) & 31u;
    const bool eight_bpp = (control & 0x80u) != 0;
    const unsigned size = (control >> 14) & 3u;
    const unsigned width = (size & 1u) ? 512u : 256u;
    const unsigned height = (size & 2u) ? 512u : 256u;
    const size_t char_base = char_bank * 0x4000u;
    const size_t screen_base = screen_bank * 0x800u;
    const size_t bytes_per_tile = eight_bpp ? 64u : 32u;
    /* GBA BG tiles/maps occupy VRAM's first 64 KiB; the upper 32 KiB are
     * reserved for OBJ data. Reject invalid maps rather than reading OBJ. */
    const size_t blocks = ((size & 1u) ? 2u : 1u) * ((size & 2u) ? 2u : 1u);
    if (char_base + 1024u * bytes_per_tile > 0x10000u ||
        screen_base + blocks * 0x800u > 0x10000u ||
        vram_length < 0x10000u) return false;
    const uint16_t backdrop = (uint16_t)((palette[0] & 0x7fffu) | 0x8000u);
    for (unsigned y = 0; y < DSI_RENDER_H; ++y) {
        uint16_t *row = out + (size_t)y * stride;
        for (unsigned x = 0; x < DSI_RENDER_W; ++x) row[x] = backdrop;
    }
    unsigned used[1024] = {0};
    for (unsigned y = 0; y < GBA_RENDER_H; ++y) {
        unsigned gy = (unsigned)((int)y + (int)scroll_y) & (height - 1u);
        unsigned ty = gy >> 3;
        uint16_t *row = out + (size_t)(y + 16u) * stride + 8;
        for (unsigned x = 0; x < GBA_RENDER_W; ++x) {
            unsigned gx = (unsigned)((int)x + (int)scroll_x) & (width - 1u);
            unsigned tx = gx >> 3;
            unsigned block_col = tx / 32u, block_row = ty / 32u;
            unsigned blocks_per_row = width / 256u;
            size_t map_index = (size_t)(block_row * blocks_per_row + block_col) * 0x800u +
                (size_t)((ty & 31u) * 32u + (tx & 31u)) * 2u;
            if (screen_base + map_index + 2u > vram_length) return false;
            unsigned entry = (unsigned)vram[screen_base + map_index] |
                ((unsigned)vram[screen_base + map_index + 1] << 8);
            unsigned tile = entry & 1023u;
            unsigned ix = (entry & 0x400u) ? 7u - (gx & 7u) : gx & 7u;
            unsigned iy = (entry & 0x800u) ? 7u - (gy & 7u) : gy & 7u;
            size_t pixel_addr = char_base + tile * bytes_per_tile + iy * (eight_bpp ? 8u : 4u) +
                (eight_bpp ? ix : ix >> 1);
            if (pixel_addr >= vram_length) return false;
            uint8_t packed = vram[pixel_addr];
            unsigned color_index = eight_bpp ? packed : (packed >> ((ix & 1u) * 4u)) & 0xfu;
            if (!eight_bpp) color_index += ((entry >> 12) & 15u) * 16u;
            /* Index 0 of every 4bpp palette is transparent on GBA. The
             * backdrop is BG palette entry 0, not bank*16. */
            if ((eight_bpp && color_index == 0) ||
                (!eight_bpp && (color_index & 15u) == 0)) color_index = 0;
            uint16_t color = (uint16_t)((palette[color_index] & 0x7fffu) | 0x8000u);
            row[x] = color;
            if (stats) {
                ++stats->pixels_drawn;
                if (color != backdrop) ++stats->pixels_not_backdrop;
                used[tile] = 1;
            }
        }
    }
    if (stats) for (size_t i = 0; i < 1024; ++i) stats->tiles_used += used[i];
    return true;
}
