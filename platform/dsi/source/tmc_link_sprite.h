#ifndef TMC_LINK_SPRITE_H
#define TMC_LINK_SPRITE_H

#include <stdbool.h>
#include <stdint.h>
#include "tmc_rom_stream.h"

/* Minimal ROM-backed Link OAM metasprite renderer for the EU GBA ROM.
 * Sprite data and palettes stay in the user's .gba; nothing is bundled.
 * This is a VISUAL DEMO, not the original player/game engine.
 */
#define TMC_LINK_FRAME_COUNT 50u
#define TMC_LINK_MAX_PIECES 8u
#define TMC_LINK_TILE_COUNT 432u

typedef struct {
    int8_t x, y;
    uint8_t flags, tile_lo, tile_hi;
} TmcLinkPiece;

typedef struct {
    uint16_t tile_base;
    uint8_t count;
    TmcLinkPiece pieces[TMC_LINK_MAX_PIECES];
} TmcLinkFrame;

typedef struct {
    uint8_t tiles[TMC_LINK_TILE_COUNT * 32u];
    uint16_t palette[4][16];
    TmcLinkFrame frames[TMC_LINK_FRAME_COUNT];
    bool ready;
} TmcLinkSprite;

bool TmcLink_Load(TmcLinkSprite *link, TmcRomReader *rom);
/* The position is Link's FEET (origin), not top-left. 0 = transparent. */
void TmcLink_Draw(const TmcLinkSprite *link, uint16_t *screen, int stride,
                  int screen_width, int screen_height,
                  int origin_x, int origin_y, unsigned frame_index, unsigned tunic);
#endif
