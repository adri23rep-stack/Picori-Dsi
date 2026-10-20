#ifndef DSI_TITLE_SPRITES_H
#define DSI_TITLE_SPRITES_H
/* DSi adapter for EU GBA OAM frame commands. No character/game behavior.
 * Frame records and their anchors are read from the owner's ROM via SD. */
#include <stdbool.h>
#include <stddef.h>
#include <stdint.h>
#include "screen.h"

typedef struct {
    int16_t x, y;
    uint16_t attr0, attr1, attr2;
} DsiTitleSpritePiece;
typedef struct {
    unsigned draw_calls, pieces, rendered_pixels, errors;
    unsigned cache_hits, cache_misses;
} DsiTitleSpriteStats;
extern DsiTitleSpriteStats gDsiTitleSpriteStats;
void DsiTitleSprites_Enable(bool enabled);
void DsiTitleSprites_Clear(void);
/* Returns false if a requested ROM record is malformed/unavailable. */
bool DsiTitleSprites_DrawDirect(uint32_t sprite, uint32_t frame, const OAMCommand *cmd);
unsigned DsiTitleSprites_Count(void);
uint32_t DsiTitleSprites_Signature(void);
/* Draws normal, non-affine 4bpp/8bpp OBJ pieces over a GBA 240x160 scene. */
bool DsiTitleSprites_Composite(uint16_t *frame, size_t stride,
    const uint8_t *vram, size_t vram_len,
    const uint16_t *obj_palette, size_t palette_len,
    uint16_t dispcnt, unsigned *drawn_pixels);
#endif
