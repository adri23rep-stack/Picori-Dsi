#ifndef DSI_TITLE_ASSETS_H
#define DSI_TITLE_ASSETS_H
#include <stdbool.h>
#include <stdint.h>
/* ROM-EU backed implementation of the ORIGINAL TitleTask's resource requests.
 * Gfx goes to GBA video/RAM shadow, palettes to GBA palette shadow.
 * E7 adds a separate renderer of those shadow bytes, not full gameplay. */
typedef struct {
    uint32_t gfx_groups, palette_groups, gfx_records, palette_records;
    uint32_t bytes_gfx, bytes_pal, lz_blocks;
    uint32_t errors;
} DsiTitleAssetStats;
extern DsiTitleAssetStats gDsiTitleAssetStats;
void DsiTitleAssets_Enable(bool enable, uint8_t language);
bool DsiTitleAssets_Active(void);
void DsiTitleAssets_ResetStats(void);
bool DsiTitleAssets_LoadGfxGroup(uint32_t group);
bool DsiTitleAssets_LoadPaletteGroup(uint32_t group);
/* Safe GBA BIOS LZ77 type 0x10 into GBA RAM/video shadow. */
bool DsiTitleAssets_UnpackLz77(uint32_t gba_src, uint32_t gba_dest, uint32_t *out_len);
#endif
