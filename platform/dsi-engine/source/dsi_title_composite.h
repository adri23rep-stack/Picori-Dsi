#ifndef DSI_TITLE_COMPOSITE_H
#define DSI_TITLE_COMPOSITE_H
#include <stdbool.h>
#include <stddef.h>
#include <stdint.h>
/* DSi backend for two regular GBA tiled BGs, with BG0 over BG1. This is
 * not game logic; the source controls/scroll/palette come from Zelda. */
typedef struct {
    unsigned foreground_pixels;
    unsigned background_pixels;
    unsigned blended_pixels;
} DsiTitleCompositeStats;
bool DsiTitleComposite_Draw(uint16_t *out, size_t stride,
    const uint8_t *vram, size_t vram_len, const uint16_t *palette, size_t pal_len,
    uint16_t fg_control, int16_t fg_x, int16_t fg_y,
    uint16_t bg_control, int16_t bg_x, int16_t bg_y,
    unsigned eva, unsigned evb, DsiTitleCompositeStats *stats);
#endif
