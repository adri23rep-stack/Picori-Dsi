#include "tmc_link_sprite.h"
#include <string.h>

/* Offsets traced to source/assets.json, data/gfx/sprite_gfx.s,
 * data/gfx/sprite_frames.s and data/gfx/frame_obj_lists.s.
 * For EU: gSprite_Link=0x13A500, gSpriteFrames_1=0x1146E0,
 * gFrameObjLists=0x2F3460, Link's normal green gPalette_14=0x5A2590.
 * First 50 frames have their own OAM piece descriptions. */
#define EU_LINK_TILES 0x0013A500u
#define EU_LINK_FRAMES 0x001146E0u
#define EU_OBJ_LISTS 0x002F3460u
#define EU_LINK_PALETTE 0x005A2590u
#define OBJ_LIST_SIZE 199561u

static unsigned Little16(const uint8_t *p) { return (unsigned)p[0] | ((unsigned)p[1] << 8); }
static uint32_t Little32(const uint8_t *p) {
    return (uint32_t)p[0] | ((uint32_t)p[1] << 8) |
           ((uint32_t)p[2] << 16) | ((uint32_t)p[3] << 24);
}

bool TmcLink_Load(TmcLinkSprite *link, TmcRomReader *rom) {
    uint8_t head[4], offs[4], palette[128];
    uint32_t sprite_table;
    if (!link || !rom || !rom->file) return false;
    link->ready = false;
    /* The EU offsets were checked with the ROM already used for v3. */
    if (rom->region != TMC_ROM_REGION_EUROPE) return false;
    if (!TmcRom_Read(rom, EU_LINK_TILES, link->tiles, sizeof(link->tiles)) ||
        !TmcRom_Read(rom, EU_LINK_PALETTE, palette, sizeof(palette)) ||
        !TmcRom_Read(rom, EU_OBJ_LISTS + 4u, offs, 4u)) return false;
    sprite_table = Little32(offs);
    if (sprite_table > OBJ_LIST_SIZE - TMC_LINK_FRAME_COUNT * 4u) return false;
    for (unsigned tunic = 0; tunic < 4; ++tunic)
        for (unsigned i = 0; i < 16; ++i)
            link->palette[tunic][i] = (uint16_t)(Little16(palette + tunic * 32u + i * 2u) & 0x7FFFu);
    for (unsigned f = 0; f < TMC_LINK_FRAME_COUNT; ++f) {
        TmcLinkFrame *frame = &link->frames[f];
        if (!TmcRom_Read(rom, EU_LINK_FRAMES + f * 4u, head, sizeof(head)) ||
            !TmcRom_Read(rom, EU_OBJ_LISTS + sprite_table + f * 4u, offs, 4u)) return false;
        frame->tile_base = (uint16_t)Little16(head + 2u);
        uint32_t offset = Little32(offs);
        uint8_t count;
        if (offset >= OBJ_LIST_SIZE ||
            !TmcRom_Read(rom, EU_OBJ_LISTS + offset, &count, 1u) ||
            count > TMC_LINK_MAX_PIECES ||
            offset + 1u + (uint32_t)count * 5u > OBJ_LIST_SIZE) return false;
        frame->count = count;
        for (unsigned j = 0; j < count; ++j) {
            uint8_t raw[5];
            if (!TmcRom_Read(rom, EU_OBJ_LISTS + offset + 1u + j * 5u, raw, 5u)) return false;
            TmcLinkPiece *p = &frame->pieces[j];
            p->x = (int8_t)raw[0];
            p->y = (int8_t)raw[1];
            p->flags = raw[2];
            p->tile_lo = raw[3];
            p->tile_hi = raw[4];
        }
    }
    link->ready = true;
    return true;
}

void TmcLink_Draw(const TmcLinkSprite *link, uint16_t *screen, int stride,
                  int sw, int sh, int px, int py, unsigned frame_index, unsigned tunic) {
    static const uint8_t widths[3][4] = {
        { 8, 16, 32, 64 }, { 16, 32, 32, 64 }, { 8, 8, 16, 32 }
    };
    static const uint8_t heights[3][4] = {
        { 8, 16, 32, 64 }, { 8, 8, 16, 32 }, { 16, 32, 32, 64 }
    };
    if (!link || !link->ready || !screen || frame_index >= TMC_LINK_FRAME_COUNT || tunic >= 4u) return;
    const TmcLinkFrame *frame = &link->frames[frame_index];
    /* OAM piece 0 is closest to camera; draw the remaining pieces first. */
    for (int j = (int)frame->count - 1; j >= 0; --j) {
        const TmcLinkPiece *p = &frame->pieces[j];
        unsigned shape = (p->flags >> 6) & 3u;
        unsigned size = (p->flags >> 4) & 3u;
        if (shape > 2) continue;
        int w = widths[shape][size], h = heights[shape][size];
        unsigned tile_base = (unsigned)frame->tile_base + p->tile_lo + ((unsigned)(p->tile_hi & 3u) << 8);
        if (tile_base + (unsigned)(w * h / 64) > TMC_LINK_TILE_COUNT) continue;
        int flip_x = !!(p->flags & 4u), flip_y = !!(p->flags & 8u);
        for (int y = 0; y < h; ++y) {
            int sy = py + p->y + y;
            if (sy < 16 || sy >= 176 || sy >= sh) continue;
            int in_y = flip_y ? h - 1 - y : y;
            for (int x = 0; x < w; ++x) {
                int sx = px + p->x + x;
                if (sx < 8 || sx >= 248 || sx >= sw) continue;
                int in_x = flip_x ? w - 1 - x : x;
                unsigned tile = tile_base + (unsigned)(in_y / 8) * (unsigned)(w / 8) + (unsigned)(in_x / 8);
                unsigned byte = tile * 32u + (unsigned)(in_y % 8) * 4u + (unsigned)(in_x % 8) / 2u;
                uint8_t packed = link->tiles[byte];
                unsigned color = (in_x & 1) ? (packed >> 4) : (packed & 15u);
                if (color) screen[sy * stride + sx] = link->palette[tunic][color] | 0x8000u;
            }
        }
    }
}
