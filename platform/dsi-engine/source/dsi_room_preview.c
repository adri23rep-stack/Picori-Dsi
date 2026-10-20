/* E25 — draw ROM-derived metatiles loaded by the existing E21/E22 bridges.
 * The GBA MapLayer fields are described in the ORIGINAL include/map.h:
 *   0x0004 mapData (room metatile indexes, initially tightly packed)
 *   0x7004 subTiles (four 8x8 GBA BG tile attributes per 16x16 metatile).
 * We deliberately do NOT call InitRoom() or touch real DS registers here.
 */
#include "dsi_room_preview.h"
#include "dsi_gba_bus.h"
#include <string.h>

/* GBA virtual addresses of ORIGINAL gMapBottom and gMapTop.
 * The E21/E22 source writes to these through the bounded GBA bus. */
#define BOTTOM_MAP       0x02025EB4u
#define TOP_MAP          0x0200B654u
#define BOTTOM_SUBTILES  0x0202CEB4u
#define TOP_SUBTILES     0x02012654u
#define BG_VRAM_LIMIT    0x10000u
#define BG_W 240u
#define BG_H 160u
#define FRAME_W 256u
#define FRAME_H 192u
#define MARGIN_X 8u
#define MARGIN_Y 16u
#define MAX_METATILES 2048u

/* Shadow memory is maintained by dsi_gba_bus.c, NOT actual GBA registers. */
extern uint8_t gVram[0x18000];
extern uint16_t gBgPltt[256];

/* Individual 8x8 GBA BG tile attributes: 10-bit character index, flips,
 * and 4-bit palette number. Keep tile #0 transparent on the upper layer. */
static bool paint_8x8(uint16_t *out, size_t stride, unsigned start_x,
                      unsigned start_y, uint16_t attr, unsigned character_bank,
                      bool transparent, DsiRoomPreviewStats *stats) {
    unsigned tile = attr & 0x3ffu;
    unsigned palette = (attr >> 12) & 15u;
    size_t offset = character_bank * 0x4000u + tile * 32u;
    if (offset + 32u > BG_VRAM_LIMIT) {
        ++stats->invalid_tiles;
        return false;
    }
    for (unsigned iy = 0; iy < 8; ++iy) {
        unsigned source_y = (attr & 0x800u) ? 7u - iy : iy;
        for (unsigned ix = 0; ix < 8; ++ix) {
            unsigned source_x = (attr & 0x400u) ? 7u - ix : ix;
            unsigned packed = gVram[offset + source_y * 4u + source_x / 2u];
            unsigned color = (packed >> (4u * (source_x & 1u))) & 15u;
            if (color == 0 && transparent) {
                ++stats->transparent_pixels;
                continue;
            }
            unsigned px = start_x + ix, py = start_y + iy;
            if (px >= BG_W || py >= BG_H) continue;
            uint16_t pixel = (uint16_t)(gBgPltt[palette * 16u + color] | 0x8000u);
            out[(size_t)(py + MARGIN_Y) * stride + px + MARGIN_X] = pixel;
            ++stats->subtile_pixels;
        }
    }
    return true;
}

static bool paint_layer(const DsiRoomMetadata *room, uint16_t *out, size_t stride,
                        bool upper, DsiRoomPreviewStats *stats) {
    /* GBA BG2 uses character bank 0; BG1 uses bank 1.
     * Original gUnk_080B77C0 (src/backgroundAnimations.c): 0x1c42, 0x1d45. */
    const unsigned character_bank = upper ? 1u : 0u;
    const uint32_t map_addr = upper ? TOP_MAP : BOTTOM_MAP;
    const uint32_t subtiles_addr = upper ? TOP_SUBTILES : BOTTOM_SUBTILES;
    const unsigned map_width = room->width / 16u;
    const unsigned map_height = room->height / 16u;
    const unsigned visible_cols = (map_width > 15u) ? 15u : map_width;
    const unsigned visible_rows = (map_height > 10u) ? 10u : map_height;
    /* The first room's source descriptors load a compact, row-major array.
     * 64-wide expansion (sub_0807C8B0) has NOT yet been run on E25. */
    for (unsigned y = 0; y < visible_rows; ++y) {
        for (unsigned x = 0; x < visible_cols; ++x) {
            const uint32_t ordinal = y * map_width + x;
            uint8_t pair[2];
            if (ordinal >= 4096u ||
                !DsiGbaBus_Read(map_addr + ordinal * 2u, pair, 2u)) return false;
            unsigned id = (unsigned)pair[0] | ((unsigned)pair[1] << 8);
            if (id >= MAX_METATILES) {
                /* Original 0x4000+ special-tile conversion has not been
                 * ported. Skip visibly rather than indexing past subtile RAM. */
                ++stats->skipped_special;
                continue;
            }
            uint8_t attributes[8];
            if (!DsiGbaBus_Read(subtiles_addr + id * 8u,
                                attributes, sizeof attributes)) return false;
            for (unsigned n = 0; n < 4; ++n) {
                uint16_t attr = (uint16_t)attributes[n * 2u] |
                                (uint16_t)((uint16_t)attributes[n * 2u + 1u] << 8);
                if (!paint_8x8(out, stride, x * 16u + (n & 1u) * 8u,
                               y * 16u + (n >> 1u) * 8u,
                               attr, character_bank, upper, stats)) return false;
            }
            ++stats->metatiles;
        }
    }
    return true;
}

bool DsiRoomPreview_Draw(const DsiRoomMetadata *room,
                         uint16_t *out, size_t stride,
                         DsiRoomPreviewStats *stats) {
    if (!room || !out || !stats || stride < FRAME_W ||
        room->width < 16u || room->height < 16u ||
        room->width > 1024u || room->height > 1024u ||
        (room->width & 15u) || (room->height & 15u) ||
        room->area != 0x22u || room->room != 0x15u) return false;
    /* Draw through a caller-owned scratch buffer, so failed calls do not
     * replace the last title frame with partially painted nonsense. */
    /* The caller owns a 256x192 screen; clear only after validating inputs. */
    memset(stats, 0, sizeof(*stats));
    uint16_t backdrop = (uint16_t)((gBgPltt[0] & 0x7fffu) | 0x8000u);
    for (unsigned y = 0; y < FRAME_H; ++y) {
        for (unsigned x = 0; x < FRAME_W; ++x)
            out[(size_t)y * stride + x] = backdrop;
    }
    if (!paint_layer(room, out, stride, false, stats) ||
        !paint_layer(room, out, stride, true, stats)) return false;
    /* A loaded but completely empty map must not claim a displayed scene. */
    for (unsigned y = MARGIN_Y; y < MARGIN_Y + BG_H; ++y)
        for (unsigned x = MARGIN_X; x < MARGIN_X + BG_W; ++x)
            if (out[(size_t)y * stride + x] != backdrop) ++stats->colored_pixels;
    return stats->metatiles > 0u && stats->colored_pixels > 0u;
}
