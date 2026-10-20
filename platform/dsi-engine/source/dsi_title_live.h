#ifndef DSI_TITLE_LIVE_H
#define DSI_TITLE_LIVE_H
/* E9: Render the current ORIGINAL title-task BG state only when Zelda changes
 * a palette group, alpha blend, scrolling or graphics. No game rules live here.
 * The caller invokes the UNMODIFIED TitleTask() once per VBlank. */
#include <stdbool.h>
#include <stddef.h>
#include <stdint.h>
#include "dsi_title_composite.h"
typedef struct {
    unsigned draws;
    unsigned palette_updates;
    unsigned gfx_updates;
    unsigned alpha_updates;
    unsigned last_pal_count;
    unsigned last_gfx_count;
    uint32_t last_sprite_signature;
    uint16_t last_alpha;
    uint16_t last_fg_control, last_bg_control;
    int16_t last_fg_x, last_fg_y, last_bg_x, last_bg_y;
    bool active;
    DsiTitleCompositeStats stats;
} DsiTitleLive;
void DsiTitleLive_Init(DsiTitleLive *s);
/* Returns false only on genuine errors. Before the forest appears, returns
 * true and *changed=false. Never writes partial output on an invalid state. */
bool DsiTitleLive_Render(DsiTitleLive *s, uint16_t *frame, size_t stride, bool *changed);
#endif
