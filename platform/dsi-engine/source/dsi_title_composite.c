#include "dsi_title_composite.h"
#include "dsi_gba_renderer.h"
#include <string.h>

typedef struct {
    const uint8_t *vram;
    size_t len;
    const uint16_t *palette;
    unsigned char_base, screen_base, width, height, tile_bytes;
    bool eight;
    int16_t x, y;
} Layer;

static bool setup(Layer *layer, const uint8_t *vram, size_t len,
    const uint16_t *pal, uint16_t cnt, int16_t x, int16_t y) {
    unsigned size = (cnt >> 14) & 3u;
    unsigned blocks = ((size & 1u) ? 2u : 1u) * ((size & 2u) ? 2u : 1u);
    layer->vram = vram;
    layer->len = len;
    layer->palette = pal;
    layer->char_base = ((cnt >> 2) & 3u) * 0x4000u;
    layer->screen_base = ((cnt >> 8) & 31u) * 0x800u;
    layer->width = (size & 1u) ? 512u : 256u;
    layer->height = (size & 2u) ? 512u : 256u;
    layer->eight = (cnt & 0x80u) != 0;
    layer->tile_bytes = layer->eight ? 64u : 32u;
    layer->x = x;
    layer->y = y;
    return layer->screen_base + blocks * 0x800u <= 0x10000u && len >= 0x10000u;
}

/* Returns transparent for palette index zero, preserving the underlying BG. */
static bool sample(const Layer *l, unsigned x, unsigned y, uint16_t *rgb) {
    unsigned gx = (x + (unsigned)(uint16_t)l->x) & (l->width - 1u);
    unsigned gy = (y + (unsigned)(uint16_t)l->y) & (l->height - 1u);
    unsigned tx = gx >> 3, ty = gy >> 3;
    size_t map = l->screen_base +
        (size_t)((ty / 32u) * (l->width / 256u) + tx / 32u) * 0x800u +
        ((ty & 31u) * 32u + (tx & 31u)) * 2u;
    if (map + 1u >= l->len) return false;
    unsigned entry = (unsigned)l->vram[map] | ((unsigned)l->vram[map+1] << 8);
    unsigned ix = (entry & 0x400u) ? 7u - (gx & 7u) : gx & 7u;
    unsigned iy = (entry & 0x800u) ? 7u - (gy & 7u) : gy & 7u;
    size_t addr = l->char_base + (entry & 1023u) * l->tile_bytes +
        iy * (l->eight ? 8u : 4u) + (l->eight ? ix : ix >> 1);
    if (addr >= 0x10000u || addr >= l->len) return false;
    unsigned index = l->vram[addr];
    if (!l->eight) index = (index >> (4u * (ix & 1u))) & 15u;
    if (!index) return false;
    if (!l->eight) index += ((entry >> 12) & 15u) * 16u;
    *rgb = l->palette[index] & 0x7fffu;
    return true;
}

static uint16_t blend(uint16_t a, uint16_t b, unsigned eva, unsigned evb) {
    unsigned out = 0;
    for (unsigned shift = 0; shift <= 10; shift += 5) {
        unsigned ca = (a >> shift) & 31u, cb = (b >> shift) & 31u;
        unsigned v = (ca * eva + cb * evb) / 16u;
        if (v > 31u) v = 31u;
        out |= v << shift;
    }
    return (uint16_t)out;
}

bool DsiTitleComposite_Draw(uint16_t *out, size_t stride,
    const uint8_t *vram, size_t len, const uint16_t *palette, size_t pal_len,
    uint16_t fg_control, int16_t fg_x, int16_t fg_y,
    uint16_t bg_control, int16_t bg_x, int16_t bg_y,
    unsigned eva, unsigned evb, DsiTitleCompositeStats *stats) {
    if (stats) memset(stats, 0, sizeof(*stats));
    if (!out || !vram || !palette || stride < DSI_RENDER_W || pal_len < 256 ||
        eva > 16u || evb > 16u) return false;
    Layer fg, bg;
    if (!setup(&fg, vram, len, palette, fg_control, fg_x, fg_y) ||
        !setup(&bg, vram, len, palette, bg_control, bg_x, bg_y)) return false;
    uint16_t backdrop = (uint16_t)(0x8000u | (palette[0] & 0x7fffu));
    for (unsigned y=0; y<DSI_RENDER_H; ++y) {
        uint16_t *row = out + y * stride;
        for (unsigned x=0; x<DSI_RENDER_W; ++x) row[x] = backdrop;
    }
    for (unsigned y=0; y<GBA_RENDER_H; ++y) {
        uint16_t *row = out + (y + 16u) * stride + 8;
        for (unsigned x=0; x<GBA_RENDER_W; ++x) {
            uint16_t a=0, b=0;
            bool fa=sample(&fg,x,y,&a), fb=sample(&bg,x,y,&b);
            if (stats) {
                stats->foreground_pixels += fa;
                stats->background_pixels += fb;
                stats->blended_pixels += fa && fb;
            }
            row[x] = (uint16_t)(0x8000u | (fa ? (fb ? blend(a,b,eva,evb) : a) :
                                                  (fb ? b : palette[0] & 0x7fffu)));
        }
    }
    return true;
}
