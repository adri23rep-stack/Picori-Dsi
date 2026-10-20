#ifndef DSI_ROOM_PREVIEW_H
#define DSI_ROOM_PREVIEW_H
/* E25 — experimental DS bitmap presentation of the ORIGINAL ROM's first
 * room metatile data. This is a renderer, not the original room initializer,
 * collision engine, entity engine or playable gameplay. No embedded assets.
 */
#include <stdbool.h>
#include <stddef.h>
#include <stdint.h>
#include "dsi_room_probe.h"

typedef struct {
    uint32_t metatiles;
    uint32_t subtile_pixels;
    uint32_t transparent_pixels;
    uint32_t skipped_special;
    uint32_t invalid_tiles;
    uint32_t colored_pixels;
} DsiRoomPreviewStats;

/* Called only after DsiRoomMaps_LoadEU and DsiRoomTiles_LoadEU succeed.
 * Renders GBA BG2 (bottom) and BG1 (top) via metatile definitions stored in
 * the original MapLayer GBA-RAM locations. 240x160 centered on 256x192 DS.
 * High bit on every pixel is set for libnds 16-bit bitmap display.
 * out[] remains unchanged on invalid metadata; failed rendering must not be
 * displayed (the caller should only DMA the buffer after a true result).
 */
bool DsiRoomPreview_Draw(const DsiRoomMetadata *room,
                         uint16_t *out, size_t stride,
                         DsiRoomPreviewStats *stats);
#endif
