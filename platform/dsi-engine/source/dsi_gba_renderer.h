#ifndef DSI_GBA_RENDERER_H
#define DSI_GBA_RENDERER_H
/* E7: Nintendo DS top-screen 16bpp framebuffer generated exclusively from
 * the GBA VRAM/palette shadow and BG control produced by original Zelda code.
 * The renderer is a display backend; it does not contain any game assets. */
#include <stdbool.h>
#include <stddef.h>
#include <stdint.h>

enum { DSI_RENDER_W = 256, DSI_RENDER_H = 192, GBA_RENDER_W = 240, GBA_RENDER_H = 160 };

typedef struct {
    uint32_t pixels_drawn;
    uint32_t pixels_not_backdrop;
    uint32_t tiles_used;
} DsiRenderStats;

/* src is the original GBA's 96 KiB video shadow, palette is its 256-entry BG
 * palette. BG CNT is the original engine gScreen.bgN.control. Only regular
 * tiled modes (4bpp/8bpp) are implemented; no sprites, fades or affine BG.
 * The 240x160 GBA view is centered on the 256x192 upper screen. */
bool DsiGbaRenderer_DrawTextBackground(uint16_t *framebuffer, size_t stride,
    const uint8_t *vram, size_t vram_size, const uint16_t *bg_palette,
    size_t palette_entries, uint16_t bg_control, int16_t scroll_x,
    int16_t scroll_y, DsiRenderStats *stats);
#endif
